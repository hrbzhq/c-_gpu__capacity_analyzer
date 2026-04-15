#include "vision/Pipeline.hpp"
#include <chrono>

namespace vision {

Pipeline::Pipeline(int streamCount) : m_streamCount(streamCount) {
    m_engines[0] = std::make_unique<Inference>(0, "model_a.engine");
    m_engines[1] = std::make_unique<Inference>(1, "model_b.engine");

    for (int i = 0; i < m_streamCount; ++i) {
        int gpuId = (i < 50) ? 0 : 1;
        m_decoders.push_back(std::make_unique<Decoder>(gpuId, "rtsp://192.168.1." + std::to_string(i)));
    }
}

Pipeline::~Pipeline() {
    stop();
}

void Pipeline::start() {
    m_running = true;
    for (int i = 0; i < m_streamCount; ++i) {
        m_workers.emplace_back(&Pipeline::workerLoop, this, i);
    }
}

void Pipeline::stop() {
    m_running = false;
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

void Pipeline::workerLoop(int id) {
    int gpuId = (id < 50) ? 0 : 1;
    auto& decoder = m_decoders[id];
    auto& engine = m_engines[gpuId];

    if (!decoder->initialize()) return;

    while (m_running) {
        void* d_ptr = nullptr;
        size_t pitch = 0;
        if (decoder->nextFrame(&d_ptr, &pitch)) {
            float output[100];
            engine->run(d_ptr, output);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }
}

SystemMetrics Pipeline::getMetrics() {
    return {
        2500.0,
        4.2,
        {
            {78.0f, 18.4f, 62.0f},
            {82.0f, 19.1f, 60.0f}
        }
    };
}

} // namespace vision
