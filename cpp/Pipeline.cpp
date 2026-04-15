#include "Pipeline.hpp"
#include <iostream>
#include <chrono>

Pipeline::Pipeline(int numStreams) : numStreams(numStreams) {
    // Initialize inference engines for both GPUs
    inferenceEngines[0] = std::make_unique<TrtInference>(0, "model_gpu0.engine");
    inferenceEngines[1] = std::make_unique<TrtInference>(1, "model_gpu1.engine");
    
    // Initialize 100 decoders (50 per GPU)
    for (int i = 0; i < numStreams; ++i) {
        int gpuId = (i < 50) ? 0 : 1;
        decoders.push_back(std::make_unique<GpuDecoder>(gpuId, "rtsp://stream_" + std::to_string(i)));
    }
}

Pipeline::~Pipeline() {
    stop();
}

void Pipeline::start() {
    running = true;
    for (int i = 0; i < numStreams; ++i) {
        int gpuId = (i < 50) ? 0 : 1;
        workerThreads.push_back(std::make_unique<std::thread>(&Pipeline::streamWorker, this, i, gpuId));
    }
}

void Pipeline::stop() {
    running = false;
    for (auto& thread : workerThreads) {
        if (thread->joinable()) thread->join();
    }
    workerThreads.clear();
}

void Pipeline::streamWorker(int streamId, int gpuId) {
    auto& decoder = decoders[streamId];
    auto& engine = inferenceEngines[gpuId];

    if (!decoder->initialize()) return;

    while (running) {
        void* d_frame = nullptr;
        size_t pitch = 0;

        // 1. Hardware-accelerated Grab (NVDEC)
        if (decoder->grabFrame(&d_frame, pitch)) {
            
            // 2. TensorRT Inference (Zero-copy)
            float output[1000]; // Example output buffer
            engine->doInference(d_frame, output, 1);
            
            // 3. Post-processing or Metadata Push
            // ...
        }

        // Throttle to target FPS if necessary
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

Pipeline::Metrics Pipeline::getMetrics() {
    // In a real app, these would be calculated from atomic counters
    return {
        2500.0f, // Total FPS (25 FPS * 100 streams)
        15.5f,   // Avg Latency
        {65.0f, 62.0f}, // GPU Util
        {18.5f, 17.8f}  // VRAM Usage
    };
}

int Pipeline::calculateMaxCapacity() {
    // Capacity calculation for Dual RTX 4090 (User Spec: 96GB Total VRAM)
    // Scenario: Frame Sampling (3 FPS per stream instead of 25 FPS)
    const float targetFps = 3.0f;
    
    // 1. VRAM Constraint: 48GB per GPU (96GB Total)
    // Each stream still needs a base VRAM footprint for decoder context and buffers
    // even at low FPS. ~100MB per stream.
    const float vramPerStreamGb = 0.10f; 
    const float totalVramGb = 96.0f; 
    int vramLimit = static_cast<int>(totalVramGb / vramPerStreamGb);

    // 2. NVDEC Constraint: 2 units per GPU, ~1000 FPS total per GPU.
    // At 3 FPS: (1000 * 2) / 3 = 666 streams.
    int nvdecLimit = static_cast<int>((1000 * 2) / targetFps); 

    // 3. Inference Constraint: RTX 4090 @ YOLOv8n
    // Total throughput is ~833 FPS per GPU.
    // At 3 FPS: (833 * 2) / 3 = 555 streams.
    int inferenceLimit = static_cast<int>((833 * 2) / targetFps);

    // 4. CPU/Network Constraint: Handling 500+ RTSP streams 
    // requires significant CPU for demuxing and network interrupts.
    int cpuLimit = 500; 

    // With 3 FPS sampling, the bottleneck shifts from NVDEC to Inference/CPU.
    return std::min({vramLimit, nvdecLimit, inferenceLimit, cpuLimit});
}
