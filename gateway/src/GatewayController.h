#pragma once
#include <httplib.h>
#include "LoadBalancer.h"

class GatewayController {
public:
    GatewayController();
    
    // Route Handlers
    void handle_register(const httplib::Request& req, httplib::Response& res);
    void handle_health(const httplib::Request& req, httplib::Response& res);

private:
    LoadBalancer load_balancer;
};
