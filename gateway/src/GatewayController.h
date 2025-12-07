#pragma once
#include <httplib.h>
#include "LoadBalancer.h"
#include <nlohmann/json.hpp>
#include <chrono>

class GatewayController {
public:
    GatewayController();
    
    // Route Handlers
    void handle_register(const httplib::Request& req, httplib::Response& res);
    void handle_health(const httplib::Request& req, httplib::Response& res);
    void handle_job_submission(const httplib::Request& req, httplib::Response& res);

    
private:
    LoadBalancer load_balancer;
    
    std::chrono::milliseconds estimate_time(const nlohmann::json &j);
    std::optional<std::string> submit_job(const Worker& worker, const nlohmann::json& payload);
};
