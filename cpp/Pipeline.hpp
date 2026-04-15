#pragma once
#include "GpuDecoder.hpp"
#include "TrtInference.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

/**
 * @brief Orchestrates 100 video streams across dual GPUs
 */
class Pipeline {
public:
    Pipeline(int numStreams = 100);
    ~Pipeline();

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

private:
    int numStreams;
    std::atomic<bool> running{false};
    std::vector<std::unique_ptr<std::thread>> workerThreads;

    // 50 streams per GPU for dual 4090 setup
    std::vector<std::unique_ptr<GpuDecoder>> decoders;
    std::unique_ptr<TrtInference> inferenceEngines[2];

    void streamWorker(int streamId, int gpuId);
};
