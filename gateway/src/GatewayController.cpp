#include "GatewayController.h"
#include <nlohmann/json.hpp>
#include <iostream>

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
