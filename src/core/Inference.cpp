#include "vision/Inference.hpp"
#include <iostream>

namespace vision {

Inference::Inference(int gpuId, const std::string& enginePath)
    : m_gpuId(gpuId), m_enginePath(enginePath) {}

Inference::~Inference() {
    if (m_context) m_context->destroy();
    if (m_engine) m_engine->destroy();
    if (m_runtime) m_runtime->destroy();
}

bool Inference::load() {
    // Real implementation would deserialize engine from disk
    return true;
}

bool Inference::run(void* d_input, float* h_output, int batchSize) {
    // Real implementation would use context->enqueueV2
    return true;
}

} // namespace vision
