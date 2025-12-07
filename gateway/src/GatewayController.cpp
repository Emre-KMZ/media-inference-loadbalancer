#include "GatewayController.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

GatewayController::GatewayController() {}

void GatewayController::handle_register(const httplib::Request& req, httplib::Response& res) {
    try {
        auto j = json::parse(req.body);
        
        Worker w;
        w.id = j.at("id").get<std::string>();
        w.port = j.at("port").get<int>();
        // In a real scenario, we might get IP from the request connection
        w.ip_address = "127.0.0.1"; 
        w.status = "HEALTHY";

        load_balancer.register_worker(w);

        res.set_content("Registered", "text/plain");
    } catch (const std::exception& e) {
        std::cerr << "Error parsing registration: " << e.what() << std::endl;
        res.status = 400;
        res.set_content("Invalid JSON", "text/plain");
    }
}

void GatewayController::handle_health(const httplib::Request& req, httplib::Response& res) {
    res.set_content("OK", "text/plain");
}

void GatewayController::handle_job_submission(const httplib::Request &req, httplib::Response &res){
    try {
        auto payload = json::parse(req.body);
        
        std::string model = payload.at("model").get<std::string>();
        std::string lora = payload.at("lora").get<std::string>();
        std::chrono::milliseconds expected_time_ms = estimate_time(payload);

        // Choose a worker
        auto worker_opt = load_balancer.get_bestfit_worker();
        if (!worker_opt) {
            res.status = 503;
            res.set_content(json{{"error", "No workers available, please try again later"}}.dump(), "application/json");
            return;
        }

        // Submit to worker
        auto job_id_opt = submit_job(*worker_opt, payload);
        if (job_id_opt) {
            json response;
            response["job_id"] = *job_id_opt;
            response["status"] = "SUBMITTED";
            response["worker_id"] = worker_opt->id; 
            
            res.status = 202; // Accepted
            res.set_content(response.dump(), "application/json");
        } else {
            res.status = 502; // Bad Gateway
            res.set_content(json{{"error", "Failed to submit job to worker"}}.dump(), "application/json");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing request: " << e.what() << std::endl;
        res.status = 400;
        res.set_content("Invalid JSON", "text/plain");
    }
}

std::optional<std::string> GatewayController::submit_job(const Worker& worker, const nlohmann::json& payload) {
    httplib::Client worker_client(worker.ip_address, worker.port);
    worker_client.set_connection_timeout(1); 
    worker_client.set_read_timeout(5);       

    auto res = worker_client.Post("/generate", payload.dump(), "application/json");
    
    if (res && res->status == 200) {
        try {
            auto j = json::parse(res->body); // if status is 200, we expect a job_id 
            if (j.contains("job_id")) {
                return j["job_id"].get<std::string>();
            }

        } catch (...) {}
    }
    
    // status is not 200 or malformed response (it either does not contain job_id or is not parsable)
    return std::nullopt;
}

std::chrono::milliseconds GatewayController::estimate_time(const nlohmann::json& j) {
    // some logic to estimate time based on model and lora
    // this is of course a placeholder
    return std::chrono::milliseconds(500);
}