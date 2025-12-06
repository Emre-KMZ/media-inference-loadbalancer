#include <httplib.h>
#include <iostream>
#include "GatewayController.h"

int main() {
    httplib::Server svr;
    GatewayController controller;

    std::cout << "Starting LoRA-Affinity Gateway..." << std::endl;

    // POST /register
    svr.Post("/register", [&](const httplib::Request& req, httplib::Response& res) {
        controller.handle_register(req, res);
    });

    // Health check for the gateway itself
    svr.Get("/health", [&](const httplib::Request& req, httplib::Response& res) {
        controller.handle_health(req, res);
    });

    std::cout << "Gateway listening on port 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}
