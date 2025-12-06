#include "LoadBalancer.h"
#include <iostream>
#include <algorithm>

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
