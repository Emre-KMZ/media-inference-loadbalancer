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

        // Submit job via LoadBalancer
        auto job_id_opt = load_balancer.dispatch_job(payload);
        
        if (job_id_opt) {
            json response;
            response["job_id"] = *job_id_opt;
            response["status"] = "SUBMITTED";
            
            res.status = 202; // Accepted
            res.set_content(response.dump(), "application/json");
        } else {
            res.status = 503; // Service Unavailable
            res.set_content(json{{"error", "No workers available or submission failed"}}.dump(), "application/json");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing request: " << e.what() << std::endl;
        res.status = 400;
        res.set_content("Invalid JSON", "text/plain");
    }
}

std::chrono::milliseconds GatewayController::estimate_time(const nlohmann::json& j) {
    // some logic to estimate time based on model and lora
    // this is of course a placeholder
    return std::chrono::milliseconds(500);
}