#pragma once
#include "Decoder.hpp"
#include "Inference.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

namespace vision {

struct SystemMetrics {
    double totalFps;
    double avgLatencyMs;
    struct GpuStats {
        float utilization;
        float vramUsedGb;
        float temp;
    } gpus[2];
};

/**
 * @brief Orchestrates 100+ video streams across multiple GPUs.
 */
class Pipeline {
public:
    Pipeline(int streamCount = 100);
    ~Pipeline();

    void start();
    void stop();
    SystemMetrics getMetrics();

private:
    int m_streamCount;
    std::atomic<bool> m_running{false};
    std::vector<std::thread> m_workers;
    
    std::vector<std::unique_ptr<Decoder>> m_decoders;
    std::unique_ptr<Inference> m_engines[2];

    void workerLoop(int id);
};

} // namespace vision
