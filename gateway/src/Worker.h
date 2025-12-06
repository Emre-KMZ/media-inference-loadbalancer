#pragma once
#include <string>
#include <vector>

struct Worker {
    std::string id;
    std::string ip_address;
    int port;
    std::string status;
    
    std::string loaded_base_model;
    std::vector<std::string> loaded_loras;
    int current_load = 0;
    bool is_healthy = true;
    long last_heartbeat = 0;
};
