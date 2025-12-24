#ifndef ROUTING_CORE_H
#define ROUTING_CORE_H

#include <thread>
#include <atomic>
#include <chrono>
#include "thread_safe_queue.h"
#include "routing_table.h"
#include "statistics.h"

class RoutingCore {
private:
    int core_id;
    ThreadSafeQueue<Packet>& input_queue;
    const RoutingTable& routing_table;
    Statistics& stats;
    std::thread worker;
    std::atomic<bool> running;
    std::atomic<uint64_t> packets_processed;
    
    static constexpr size_t CONGESTION_THRESHOLD = 8000;
    
    void worker_loop() {
        while (running.load()) {
            Packet packet;
            if (input_queue.pop(packet)) {
                process_packet(packet);
            }
        }
    }
    
    void process_packet(Packet& packet) {
        packets_processed++;
        stats.total_packets++;
        
        auto current_time = std::chrono::steady_clock::now().time_since_epoch();
        uint64_t now_us = std::chrono::duration_cast<std::chrono::microseconds>(current_time).count();
        packet.processing_start_time = now_us;
        
        if (input_queue.size() > CONGESTION_THRESHOLD) {
            stats.packets_dropped++;
            stats.congestion_drops++;
            return;
        }
        
        int output_port = routing_table.lookup(packet.dest_ip);
        
        if (output_port == -1) {
            stats.packets_dropped++;
            stats.no_route_drops++;
            return;
        }
        
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        
        stats.packets_forwarded++;
        
        uint64_t latency = packet.processing_start_time - packet.arrival_time;
        stats.record_latency(latency);
    }
    
public:
    RoutingCore(int id, ThreadSafeQueue<Packet>& queue, 
                const RoutingTable& table, Statistics& statistics)
        : core_id(id), input_queue(queue), routing_table(table), 
          stats(statistics), running(false), packets_processed(0) {}
    
    void start() {
        running = true;
        worker = std::thread(&RoutingCore::worker_loop, this);
    }
    
    void stop() {
        running = false;
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    uint64_t get_packets_processed() const {
        return packets_processed.load();
    }
    
    int get_core_id() const {
        return core_id;
    }
    
    ~RoutingCore() {
        stop();
    }
};

#endif