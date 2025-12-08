#include "LoadBalancer.h"
#include <iostream>
#include <algorithm>
#include <httplib.h>

LoadBalancer::LoadBalancer() {}

void LoadBalancer::register_worker(const Worker& worker) {
    std::lock_guard<std::mutex> lock(workers_mutex);
    
    auto it = std::find_if(workers.begin(), workers.end(), 
        [&worker](const Worker& w) { return w.id == worker.id; });

    if (it != workers.end()) {
        // Update existing worker
        *it = worker;
        std::cout << "[LoadBalancer] Worker Updated: " << worker.id << " on port " << worker.port << std::endl;
    } else {
        // Add new worker
        workers.push_back(worker);
        std::cout << "[LoadBalancer] Worker Registered: " << worker.id << " on port " << worker.port << std::endl;
    }
    
    std::cout << "[LoadBalancer] Total Workers: " << workers.size() << std::endl;
}

std::vector<Worker> LoadBalancer::get_workers() {
    std::lock_guard<std::mutex> lock(workers_mutex);
    return workers;
}
// score of worker for the payload
int LoadBalancer::score_worker_for_payload(const Worker& worker, const nlohmann::json& payload) {
    int score = 0;
    try {
        std::string model = payload.at("model").get<std::string>();
        std::string lora = payload.at("lora").get<std::string>();

        if (worker.loaded_models.count(model) > 0) {
            score += 100;
            if (worker.loaded_models.at(model).count(lora) > 0) {
                score += 40;
            }
        } else if (worker.vram_total - worker.vram_used >= 2048) { // assuming each model+LoRA needs 2GB VRAM, better approach would be storing actual VRAM requirements for each model
            score += 20; // some score for having enough VRAM
        }

        
        score += std::max(0, worker.max_load - worker.current_load); // prefer less loaded workers

    } catch (const std::exception& e) {
        std::cerr << "Error scoring worker " << worker.id << ": " << e.what() << std::endl;
        return 0;
    }

    return score;
}

// This function both selects the best-fit worker and increments its load
// !Caller of this function is responsible for decrementing load if job submission fails
std::optional<Worker> LoadBalancer::get_bestfit_worker_and_increment_load(nlohmann::json payload) {
    std::lock_guard<std::mutex> lock(workers_mutex);
    
    int best_score = -1;
    Worker* best_worker = nullptr;

    for (auto& worker : workers) {
        if (!worker.is_healthy || worker.current_load >= worker.max_load) {
            continue; // skip unhealthy or overloaded workers
        }

        int score = score_worker_for_payload(worker, payload);
        if (score > best_score) {
            best_score = score;
            best_worker = &worker;
        }
    }

    if (best_worker) {
        best_worker->current_load++; // i didnt like my design here
        // this is an unintended/non-expicit side-effect,
        // feels like in the future someone would just call this function to peek the best
        // worker without actually dispatching a job and it would mess up the load balancing
        // for now i will just rename the funtion from "get_bestfit_worker" to "get_bestfit_worker_and_increment_load"
        // hyrumslaw.com
        return *best_worker;
    }
    return std::nullopt;
}
std::optional<std::string> LoadBalancer::dispatch_job(const nlohmann::json& payload) {
    auto worker_opt = get_bestfit_worker_and_increment_load(payload);
    if (!worker_opt) {
        return std::nullopt;
    }
    
    auto job_id = submit_to_worker(*worker_opt, payload);
    
    if (!job_id) {
        // Rollback load increment if submission failed
        std::lock_guard<std::mutex> lock(workers_mutex);
        auto it = std::find_if(workers.begin(), workers.end(), 
            [&](const Worker& w) { return w.id == worker_opt->id; });
            
        if (it != workers.end() && it->current_load > 0) {
            it->current_load--;
        }
    }
    
    return job_id;
}

std::optional<std::string> LoadBalancer::submit_to_worker(const Worker& worker, const nlohmann::json& payload) {
    httplib::Client worker_client(worker.ip_address, worker.port);
    worker_client.set_connection_timeout(1); 
    worker_client.set_read_timeout(5);       

    auto res = worker_client.Post("/generate", payload.dump(), "application/json");
    
    if (res && res->status == 200) {
        try {
            auto j = nlohmann::json::parse(res->body); 
            if (j.contains("job_id")) {
                return j["job_id"].get<std::string>();
            }
        } catch (...) {}
    }
    
    return std::nullopt;
}
