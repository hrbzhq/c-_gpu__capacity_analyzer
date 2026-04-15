#pragma once
#include <NvInfer.h>
#include <cuda_runtime.h>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief TensorRT Inference Engine Wrapper
 * Optimized for high-throughput batch processing.
 */
class TrtInference {
public:
    TrtInference(int gpuId, const std::string& enginePath);
    ~TrtInference();

    // Load and deserialize the engine
    bool loadEngine();

    // Build engine from ONNX model (YOLOv8/v10/v11)
    bool buildEngineFromOnnx(const std::string& onnxPath, int maxBatchSize);

    // Perform inference on a batch of frames already in GPU memory
    // Zero-copy approach: input is a CUDA device pointer
    bool doInference(void* d_input, float* h_output, int batchSize);

private:
    int gpuId;
    std::string enginePath;

    nvinfer1::IRuntime* runtime = nullptr;
    nvinfer1::ICudaEngine* engine = nullptr;
    nvinfer1::IExecutionContext* context = nullptr;

    cudaStream_t stream;
    std::vector<void*> buffers; // GPU buffers for input/output
};
