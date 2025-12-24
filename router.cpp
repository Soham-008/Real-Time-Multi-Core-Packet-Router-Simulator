//
// Created by Soham Kulkarni on 17/12/25.
//
#include "router.h"
#include <iostream>
#include <iomanip>

Router::Router(int num_cores, LoadBalancingStrategy strategy)
    : num_cores(num_cores), input_queue(10000),
      load_balance_strategy(strategy), round_robin_counter(0) {

    for (int i = 0; i < num_cores; i++) {
        cores.push_back(
            std::make_unique<RoutingCore>(i, input_queue, routing_table, stats)
        );
    }
}

Router::~Router() {
    stop();
}

void Router::add_route(uint32_t dest_ip, int output_port) {
    routing_table.add_route(dest_ip, output_port);
}

void Router::start() {
    std::cout << "Starting " << num_cores << " routing cores...\n";
    for (auto& core : cores) {
        core->start();
    }
}

void Router::stop() {
    std::cout << "Stopping routing cores...\n";
    input_queue.set_shutdown();
    for (auto& core : cores) {
        core->stop();
    }
}

bool Router::inject_packet(const Packet& packet) {
    return input_queue.push(packet);
}

void Router::run_simulation(uint64_t num_packets, uint64_t arrival_rate_us) {
    std::cout << "\n=== Starting Simulation ===\n";
    std::cout << "Packets to generate: " << num_packets << "\n";
    std::cout << "Arrival rate: one packet every " << arrival_rate_us << " μs\n";
    std::cout << "Load balancing: ";

    switch (load_balance_strategy) {
        case ROUND_ROBIN:
            std::cout << "Round Robin\n";
            break;
        case LEAST_LOADED:
            std::cout << "Least Loaded\n";
            break;
        case HASH_BASED:
            std::cout << "Hash Based\n";
            break;
    }

    std::cout << "\nGenerating packets...\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> ip_dist(0x08080808, 0x0A0000FF);
    std::uniform_int_distribution<uint8_t> priority_dist(0, 2);
    std::uniform_int_distribution<uint32_t> size_dist(64, 1500);

    auto start_time = std::chrono::steady_clock::now();

    for (uint64_t i = 0; i < num_packets; i++) {
        auto current_time = std::chrono::steady_clock::now();
        uint64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - start_time
        ).count();

        uint32_t dest_ip = ip_dist(gen);
        uint8_t priority = priority_dist(gen);
        uint32_t size = size_dist(gen);

        Packet packet(i, 0xC0A80101, dest_ip, elapsed_us, priority, size);

        if (!inject_packet(packet)) {
            stats.packets_dropped++;
            stats.congestion_drops++;
        }

        if ((i + 1) % 10000 == 0) {
            std::cout << "Generated " << (i + 1) << " packets...\r" << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(arrival_rate_us));
    }

    std::cout << "\nWaiting for packets to be processed...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto end_time = std::chrono::steady_clock::now();
    uint64_t total_time_us = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time
    ).count();

    double throughput = (stats.packets_forwarded.load() * 1000000.0) / total_time_us;

    std::cout << "\n=== Simulation Complete ===\n";
    std::cout << "Total simulation time: " << (total_time_us / 1000000.0) << " seconds\n";
    std::cout << "Throughput: " << std::fixed << std::setprecision(2)
              << throughput << " packets/second\n";
}

Statistics& Router::get_statistics() {
    return stats;
}

void Router::print_statistics() const {
    stats.print_summary();
}

void Router::print_core_statistics() const {
    std::cout << "\n=== Per-Core Statistics ===\n";
    for (const auto& core : cores) {
        std::cout << "Core " << core->get_core_id() << ": "
                  << core->get_packets_processed() << " packets processed\n";
    }
    std::cout << "===========================\n";
}