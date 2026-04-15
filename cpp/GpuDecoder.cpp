#include "GpuDecoder.hpp"
#include <iostream>

GpuDecoder::GpuDecoder(int gpuId, const std::string& rtspUrl) 
    : gpuId(gpuId), url(rtspUrl) {}

GpuDecoder::~GpuDecoder() {
    // Cleanup NVDEC resources
    if (hDecoder) cuvidDestroyDecoder(hDecoder);
    if (cuContext) cuCtxDestroy(cuContext);
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
    
    // Mock implementation for structure:
    // cuvidDecodePicture(hDecoder, ...);
    // CUVIDPROCPARAMS params = {};
    // cuvidMapVideoFrame(hDecoder, index, (unsigned long long*)d_framePtr, (unsigned int*)&pitch, &params);
    
    return true; 
}
