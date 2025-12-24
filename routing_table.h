#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <unordered_map>
#include <shared_mutex>
#include <cstdint>
#include <vector>

class RoutingTable {
private:
    std::unordered_map<uint32_t, int> routes;
    mutable std::shared_mutex mtx;
    
public:
    void add_route(uint32_t dest_ip, int output_port) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        routes[dest_ip] = output_port;
    }
    
    int lookup(uint32_t dest_ip) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto it = routes.find(dest_ip);
        if (it != routes.end()) {
            return it->second;
        }
        return -1;
    }
    
    bool remove_route(uint32_t dest_ip) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        return routes.erase(dest_ip) > 0;
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        routes.clear();
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return routes.size();
    }
    
    std::vector<std::pair<uint32_t, int>> get_all_routes() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return std::vector<std::pair<uint32_t, int>>(routes.begin(), routes.end());
    }
};

#endif