#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include "routing_core.h"
#include "routing_table.h"
#include "thread_safe_queue.h"
#include "statistics.h"
#include "packet.h"

enum LoadBalancingStrategy {
    ROUND_ROBIN,
    LEAST_LOADED,
    HASH_BASED
};

class Router {
private:
    int num_cores;
    RoutingTable routing_table;
    ThreadSafeQueue<Packet> input_queue;
    Statistics stats;
    std::vector<std::unique_ptr<RoutingCore>> cores;
    LoadBalancingStrategy load_balance_strategy;
    std::atomic<int> round_robin_counter;
    
public:
    Router(int num_cores, LoadBalancingStrategy strategy = ROUND_ROBIN);
    ~Router();
    
    void add_route(uint32_t dest_ip, int output_port);
    void start();
    void stop();
    bool inject_packet(const Packet& packet);
    void run_simulation(uint64_t num_packets, uint64_t arrival_rate_us);
    Statistics& get_statistics();
    void print_statistics() const;
    void print_core_statistics() const;
};

#endif