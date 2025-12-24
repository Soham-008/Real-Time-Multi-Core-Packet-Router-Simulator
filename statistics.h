#ifndef STATISTICS_H
#define STATISTICS_H

#include <atomic>
#include <vector>
#include <algorithm>
#include <mutex>
#include <iostream>  // ADDED: Missing include

struct Statistics {
    std::atomic<uint64_t> total_packets{0};
    std::atomic<uint64_t> packets_forwarded{0};
    std::atomic<uint64_t> packets_dropped{0};
    std::atomic<uint64_t> congestion_drops{0};
    std::atomic<uint64_t> no_route_drops{0};

    // FIXED: Removed const from mutex (mutexes cannot be const)
    mutable std::mutex latency_mutex;
    std::vector<uint64_t> latencies;
    
    void record_latency(uint64_t latency) {
        std::lock_guard<std::mutex> lock(latency_mutex);
        latencies.push_back(latency);
    }
    
    double get_average_latency() const {
        std::lock_guard<std::mutex> lock(latency_mutex);
        if (latencies.empty()) return 0.0;
        uint64_t sum = 0;
        for (auto lat : latencies) sum += lat;
        return static_cast<double>(sum) / latencies.size();
    }
    
    uint64_t get_percentile_latency(double percentile) const {
        std::lock_guard<std::mutex> lock(latency_mutex);
        if (latencies.empty()) return 0;
        
        std::vector<uint64_t> sorted_latencies = latencies;
        std::sort(sorted_latencies.begin(), sorted_latencies.end());
        
        size_t index = static_cast<size_t>(percentile * sorted_latencies.size());
        if (index >= sorted_latencies.size()) {
            index = sorted_latencies.size() - 1;
        }
        return sorted_latencies[index];
    }
    
    void reset() {
        total_packets = 0;
        packets_forwarded = 0;
        packets_dropped = 0;
        congestion_drops = 0;
        no_route_drops = 0;
        
        std::lock_guard<std::mutex> lock(latency_mutex);
        latencies.clear();
    }
    
    void print_summary() const {
        std::cout << "\n=== Routing Statistics ===\n";
        std::cout << "Total packets:        " << total_packets.load() << "\n";
        std::cout << "Packets forwarded:    " << packets_forwarded.load() << "\n";
        std::cout << "Packets dropped:      " << packets_dropped.load() << "\n";
        std::cout << "  - No route:         " << no_route_drops.load() << "\n";
        std::cout << "  - Congestion:       " << congestion_drops.load() << "\n";
        
        if (!latencies.empty()) {
            std::cout << "\nLatency Statistics:\n";
            std::cout << "  Average:            " << get_average_latency() << " μs\n";
            std::cout << "  Median (p50):       " << get_percentile_latency(0.50) << " μs\n";
            std::cout << "  p95:                " << get_percentile_latency(0.95) << " μs\n";
            std::cout << "  p99:                " << get_percentile_latency(0.99) << " μs\n";
        }
        
        double drop_rate = (total_packets.load() > 0) 
            ? (100.0 * packets_dropped.load() / total_packets.load()) 
            : 0.0;
        std::cout << "\nDrop rate:            " << drop_rate << "%\n";
        std::cout << "=========================\n";
    }
};

#endif