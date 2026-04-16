#pragma once
#include "GpuDecoder.hpp"
#include "TrtInference.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>

/**
 * @brief Orchestrates video streams across dual GPUs with 4-model TensorRT array
 */
class Pipeline {
public:
    // Constructor takes vector of engine paths (4 models for array)
    Pipeline(const std::vector<std::string>& enginePaths, 
             const std::string& cameraApiUrl = "http://192.168.8.191:9061/api/cameras/list");
    ~Pipeline();

    // Fetch camera URLs from HTTP API
    bool fetchCameraUrls();
    
    void start();
    void stop();

    // Metrics for monitoring
    struct Metrics {
        float totalFps;
        float avgLatencyMs;
        float gpuUtilization[2];
        float vramUsageGb[2];
    };
    Metrics getMetrics();

    // Calculate maximum stable streams based on current hardware
    int calculateMaxCapacity();
    
    // Get number of streams (cameras)
    int getNumStreams() const { return numStreams; }

private:
    int numStreams;
    std::string cameraApiUrl;
    std::vector<std::string> cameraUrls;
    std::vector<std::string> enginePaths;
    std::atomic<bool> running{false};
    std::vector<std::unique_ptr<std::thread>> workerThreads;

    // 4-model array for multi-model inference
    std::vector<std::unique_ptr<GpuDecoder>> decoders;
    std::vector<std::unique_ptr<TrtInference>> inferenceEngines;

    void streamWorker(int streamId, int gpuId, int engineIdx);
};
