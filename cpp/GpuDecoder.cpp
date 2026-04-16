#include "GpuDecoder.hpp"
#include <iostream>
#include <cuda_runtime.h>

GpuDecoder::GpuDecoder(int gpuId, const std::string& rtspUrl) 
    : gpuId(gpuId), url(rtspUrl), d_frameBuffer(nullptr), frameBufferSize(0) {}

GpuDecoder::~GpuDecoder() {
    // Cleanup NVDEC resources
    if (hDecoder) cuvidDestroyDecoder(hDecoder);
    if (cuContext) cuCtxDestroy(cuContext);
    if (d_frameBuffer) cudaFree(d_frameBuffer);
}

bool GpuDecoder::initialize() {
    // 1. Initialize CUDA Context on specific GPU
    cuInit(0);
    CUdevice cuDevice;
    cuDeviceGet(&cuDevice, gpuId);
    cuCtxCreate(&cuContext, 0, cuDevice);

    // 2. Setup NVDEC Decoder
    // In a real implementation, we would parse the bitstream first to get width/height
    width = 1920;
    height = 1080;

    CUVIDDECODECREATEINFO decodeInfo = {};
    decodeInfo.CodecType = cudaVideoCodec_H264;
    decodeInfo.ulWidth = width;
    decodeInfo.ulHeight = height;
    decodeInfo.ulNumDecodeSurfaces = 20;
    decodeInfo.ChromaFormat = cudaVideoChromaFormat_420;
    decodeInfo.OutputFormat = cudaVideoSurfaceFormat_NV12;
    decodeInfo.bitDepthMinus8 = 0;
    decodeInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;
    decodeInfo.ulTargetWidth = width;
    decodeInfo.ulTargetHeight = height;

    CUresult res = cuvidCreateDecoder(&hDecoder, &decodeInfo);
    return (res == CUDA_SUCCESS);
}

bool GpuDecoder::grabFrame(void** d_framePtr, size_t& pitch) {
    // This would involve:
    // 1. Demuxing RTSP stream (e.g. using FFmpeg)
    // 2. Passing packets to cuvidParseVideoData
    // 3. Handling callbacks to decode and map surfaces
    
    // For now: allocate dummy GPU buffer for testing (1920x1080 NV12 = ~3MB)
    size_t requiredSize = 1920 * 1080 * 3 / 2; // NV12 format
    
    if (!d_frameBuffer) {
        cudaError_t err = cudaMalloc(&d_frameBuffer, requiredSize);
        if (err != cudaSuccess) {
            std::cerr << "Failed to allocate GPU memory for frame buffer" << std::endl;
            return false;
        }
        frameBufferSize = requiredSize;
        // Initialize with zeros
        cudaMemset(d_frameBuffer, 0, requiredSize);
    }
    
    *d_framePtr = d_frameBuffer;
    pitch = 1920; // row pitch
    
    return true; 
}
