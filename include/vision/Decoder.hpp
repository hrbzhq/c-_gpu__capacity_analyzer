#pragma once
#include <cuda_runtime.h>
#include <nvcuvid.h>
#include <string>

namespace vision {

/**
 * @brief Hardware-accelerated decoder using NVIDIA NVDEC.
 * Decodes H.264/H.265 streams directly into GPU memory.
 */
class Decoder {
public:
    Decoder(int gpuId, const std::string& source);
    ~Decoder();

    bool initialize();
    bool nextFrame(void** d_ptr, size_t* pitch);

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    int m_gpuId;
    std::string m_source;
    int m_width = 0;
    int m_height = 0;

    CUvideodecoder m_hDecoder = nullptr;
    CUcontext m_cuContext = nullptr;
};

} // namespace vision
