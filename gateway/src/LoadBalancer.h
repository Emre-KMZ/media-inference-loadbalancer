#pragma once
#include "Worker.h"
#include <vector>
#include <mutex>
#include <optional>

class LoadBalancer {
public:
    LoadBalancer();
    
    // Registers a new worker or updates an existing one
    void register_worker(const Worker& worker);
    
    // Returns a copy of the current worker list (thread-safe)
    std::vector<Worker> get_workers();

private:
    std::vector<Worker> workers;
    std::mutex workers_mutex;
};
