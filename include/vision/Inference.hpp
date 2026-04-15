#pragma once
#include <NvInfer.h>
#include <cuda_runtime.h>
#include <string>
#include <vector>

namespace vision {

/**
 * @brief TensorRT Inference Engine.
 * Optimized for high-throughput batch processing on NVIDIA GPUs.
 */
class Inference {
public:
    Inference(int gpuId, const std::string& enginePath);
    ~Inference();

    bool load();
    bool run(void* d_input, float* h_output, int batchSize = 1);

private:
    int m_gpuId;
    std::string m_enginePath;

    nvinfer1::IRuntime* m_runtime = nullptr;
    nvinfer1::ICudaEngine* m_engine = nullptr;
    nvinfer1::IExecutionContext* m_context = nullptr;
    cudaStream_t m_stream;
};

} // namespace vision
