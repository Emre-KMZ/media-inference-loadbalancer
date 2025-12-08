#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>

struct Worker {
    std::string id;
    std::string ip_address;
    int port;
    std::string status;
    
    int max_load = 7;

    // for now want to have the dumb assumption that each worker can run as much models as they fit in vram
    // why it is dumb:
    // - Compute contention: even if vram is enough, the worker might be overloaded in terms of compute
    // - Memory fragmentation: vram might be enough in total, but not enough contiguous memory for a new model
    std::map<std::string, std::set<std::string>> loaded_models; // map of base_model_id -> set of loaded lora_ids)
    // it would be also nice if we had a clever eviction policy, but for now i will ignore it since it is not the point of this exercise
    
    // in megabytes
    uint64_t vram_used = 0; 
    // theres an unenforced invariant that this variable should be equal to the sum of vram usages of loaded models
    // this made me think of Yaron Minsky's advice: "make illegal states unrepresentable", what would be a better design here?
    uint64_t vram_total = 0;

    int current_load = 0; // number of active jobs
    bool is_healthy = true;
    long last_heartbeat = 0; // inactive for now
};
