#include "Pipeline.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting VisionStream X100 High-Performance Video System..." << std::endl;
    std::cout << "Target: 100 Streams | Dual RTX 4090 | Ubuntu 24.04" << std::endl;

    try {
        // Initialize pipeline with 100 streams
        Pipeline pipeline(100);
        pipeline.start();

        // Main monitoring loop
        while (true) {
            auto metrics = pipeline.getMetrics();
            
            printf("\r[SYSTEM] Total FPS: %.2f | Avg Latency: %.2fms | GPU0: %.1f%% | GPU1: %.1f%%", 
                   metrics.totalFps, metrics.avgLatencyMs, 
                   metrics.gpuUtilization[0], metrics.gpuUtilization[1]);
            fflush(stdout);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
