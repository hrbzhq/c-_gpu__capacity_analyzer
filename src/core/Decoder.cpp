#include "vision/Decoder.hpp"
#include <iostream>

namespace vision {

Decoder::Decoder(int gpuId, const std::string& source) 
    : m_gpuId(gpuId), m_source(source) {}

Decoder::~Decoder() {
    if (m_hDecoder) cuvidDestroyDecoder(m_hDecoder);
    if (m_cuContext) cuCtxDestroy(m_cuContext);
}

bool Decoder::initialize() {
    // Real implementation would use cuvidCreateDecoder after parsing bitstream
    m_width = 1920;
    m_height = 1080;
    return true;
}

bool Decoder::nextFrame(void** d_ptr, size_t* pitch) {
    // Mock: In real code, this maps a decoded surface to a CUDA pointer
    return true;
}

} // namespace vision
