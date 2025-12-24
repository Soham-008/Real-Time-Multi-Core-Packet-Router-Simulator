#include <iostream>
#include <iomanip>
#include "router.h"

void print_header() {
    std::cout << R"(
╔═══════════════════════════════════════════════════╗
║   Multi-Core Packet Router Simulator              ║
║   High-Performance Discrete-Event Simulation      ║
╚═══════════════════════════════════════════════════╝
)" << "\n";
}

void run_benchmark(int num_cores, uint64_t num_packets) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "BENCHMARK: " << num_cores << " cores, "
              << num_packets << " packets\n";
    std::cout << std::string(60, '=') << "\n";

    Router router(num_cores, ROUND_ROBIN);

    router.add_route(0x08080808, 0);
    router.add_route(0x08080844, 1);
    router.add_route(0xC0A80001, 2);
    router.add_route(0x0A000001, 3);

    for (uint32_t i = 0x0A000002; i <= 0x0A0000FF; i++) {
        router.add_route(i, i % 4);
    }

    router.start();

    router.run_simulation(num_packets, 10);

    router.stop();

    router.print_statistics();
    router.print_core_statistics();
}

void run_scalability_test() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "SCALABILITY TEST\n";
    std::cout << std::string(60, '=') << "\n";

    int core_counts[] = {1, 2, 4, 8};
    uint64_t test_packets = 50000;

    std::cout << "\nTesting with " << test_packets << " packets\n\n";
    std::cout << std::left << std::setw(10) << "Cores"
              << std::setw(20) << "Throughput (pps)"
              << std::setw(15) << "Speedup\n";
    std::cout << std::string(45, '-') << "\n";

    double baseline_throughput = 0;

    for (int cores : core_counts) {
        Router router(cores, ROUND_ROBIN);

        for (uint32_t i = 0x0A000001; i <= 0x0A0000FF; i++) {
            router.add_route(i, i % 4);
        }

        router.start();

        auto start = std::chrono::steady_clock::now();
        router.run_simulation(test_packets, 5);
        auto end = std::chrono::steady_clock::now();

        router.stop();

        uint64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start
        ).count();

        double throughput = (test_packets * 1000000.0) / elapsed_us;

        if (cores == 1) {
            baseline_throughput = throughput;
        }

        double speedup = throughput / baseline_throughput;

        std::cout << std::left << std::setw(10) << cores
                  << std::setw(20) << std::fixed << std::setprecision(2) << throughput
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x\n";
    }
}

int main(int argc, char* argv[]) {
    print_header();

    if (argc > 1 && std::string(argv[1]) == "--scalability") {
        run_scalability_test();
        return 0;
    }

    int num_cores = 4;
    uint64_t num_packets = 100000;

    if (argc > 1) {
        num_cores = std::stoi(argv[1]);
    }
    if (argc > 2) {
        num_packets = std::stoull(argv[2]);
    }

    std::cout << "Configuration:\n";
    std::cout << "  Cores: " << num_cores << "\n";
    std::cout << "  Packets: " << num_packets << "\n\n";

    run_benchmark(num_cores, num_packets);

    std::cout << "\n✓ Simulation completed successfully!\n\n";
    std::cout << "To run scalability test: ./packet_router_simulator --scalability\n";
    std::cout << "To specify cores/packets: ./packet_router_simulator <cores> <packets>\n";

    return 0;
}