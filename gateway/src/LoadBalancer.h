#pragma once
#include "Worker.h"
#include <vector>
#include <mutex>
#include <optional>
#include <nlohmann/json.hpp>

class LoadBalancer {
public:
    LoadBalancer();
    
    // Registers a new worker or updates an existing one
    void register_worker(const Worker& worker);
    
    // Returns a copy of the current worker list (thread-safe)
    std::vector<Worker> get_workers();

    std::optional<Worker> get_bestfit_worker_and_increment_load(nlohmann::json payload);

    // Finds a worker and submits the job
    std::optional<std::string> dispatch_job(const nlohmann::json& payload);
    
    private:
    std::vector<Worker> workers;
    std::mutex workers_mutex;
    
    // Selects the best-fit worker for a new job
    // Scores a worker for a given payload (to be used in best-fit selection)
    int score_worker_for_payload(const Worker &worker, const nlohmann::json &payload);

    // Helper to actually send the request
    std::optional<std::string> submit_to_worker(const Worker& worker, const nlohmann::json& payload);
};
