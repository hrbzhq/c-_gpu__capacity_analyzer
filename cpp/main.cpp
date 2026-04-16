#include "Pipeline.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

int main() {
    std::cout << "Starting VisionStream X100 High-Performance Video System..." << std::endl;
    std::cout << "Features: 4-Model TensorRT Array | HTTP Camera API | Dual RTX 4090" << std::endl;

    try {
        // Define 4 TensorRT engine model paths (model array)
        std::vector<std::string> enginePaths = {
            "c-_gpu__capacity_analyzer-main/yolo_models/20251027_205844.engine",
            "c-_gpu__capacity_analyzer-main/yolo_models/20251104_172108.engine",
            "c-_gpu__capacity_analyzer-main/yolo_models/20251113_160034.engine",
            "c-_gpu__capacity_analyzer-main/yolo_models/s20251204_162540.engine"
        };

        // Camera API URL
        const std::string cameraApiUrl = "http://192.168.8.191:9061/api/cameras/list";

        // Initialize pipeline with 4-model array
        Pipeline pipeline(enginePaths, cameraApiUrl);

        // Fetch camera URLs from HTTP API
        std::cout << "Fetching camera URLs from: " << cameraApiUrl << std::endl;
        if (!pipeline.fetchCameraUrls()) {
            std::cerr << "Failed to fetch camera URLs from API. Exiting." << std::endl;
            return 1;
        }

        std::cout << "Loaded " << pipeline.getNumStreams() << " streams. Starting pipeline..." << std::endl;
        pipeline.start();

        // Main monitoring loop
        while (true) {
            auto metrics = pipeline.getMetrics();
            
            printf("\r[SYSTEM] Streams: %d | Total FPS: %.2f | Avg Latency: %.2fms | GPU0: %.1f%% | GPU1: %.1f%%", 
                   pipeline.getNumStreams(), metrics.totalFps, metrics.avgLatencyMs, 
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
