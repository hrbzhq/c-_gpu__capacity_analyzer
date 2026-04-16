#pragma once
#include <cuda_runtime.h>
#include <nvcuvid.h>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief High-performance GPU Decoder using NVIDIA Video Codec SDK (NVDEC)
 * Handles hardware-accelerated decoding directly into GPU memory.
 */
class GpuDecoder {
public:
    GpuDecoder(int gpuId, const std::string& rtspUrl);
    ~GpuDecoder();

    // Initialize the decoder and connection
    bool initialize();

    // Grab the next frame directly in GPU memory (CUDA Device Pointer)
    // Returns true if successful, false on EOF or error
    bool grabFrame(void** d_framePtr, size_t& pitch);

    // Get stream metadata
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int gpuId;
    std::string url;
    int width = 0;
    int height = 0;

    // NVDEC specific handles
    CUvideodecoder hDecoder = nullptr;
    CUvideoparser hParser = nullptr;
    CUcontext cuContext = nullptr;

    // Internal buffer management
    void* d_frameBuffer = nullptr;
    size_t frameBufferSize = 0;   
};
