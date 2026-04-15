#include "TrtInference.hpp"
#include <fstream>
#include <iostream>
#include <NvOnnxParser.h>

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING) std::cout << msg << std::endl;
    }
} gLogger;

TrtInference::TrtInference(int gpuId, const std::string& enginePath)
    : gpuId(gpuId), enginePath(enginePath) {}

TrtInference::~TrtInference() {
    if (context) context->destroy();
    if (engine) engine->destroy();
    if (runtime) runtime->destroy();
}

bool TrtInference::loadEngine() {
    std::ifstream file(enginePath, std::ios::binary);
    if (!file.good()) return false;

    file.seekg(0, file.end);
    size_t size = file.tellg();
    file.seekg(0, file.beg);

    char* data = new char[size];
    file.read(data, size);
    file.close();

    runtime = nvinfer1::createInferRuntime(gLogger);
    engine = runtime->deserializeCudaEngine(data, size);
    delete[] data;

    if (!engine) return false;
    context = engine->createExecutionContext();
    
    cudaSetDevice(gpuId);
    cudaStreamCreate(&stream);
    
    return true;
}

bool TrtInference::buildEngineFromOnnx(const std::string& onnxPath, int maxBatchSize) {
    auto builder = nvinfer1::createInferBuilder(gLogger);
    const auto explicitBatch = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    auto network = builder->createNetworkV2(explicitBatch);
    auto config = builder->createBuilderConfig();
    auto parser = nvonnxparser::createParser(*network, gLogger);

    if (!parser->parseFromFile(onnxPath.c_str(), static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
        std::cerr << "Failed to parse ONNX file" << std::endl;
        return false;
    }

    config->setMaxWorkspaceSize(1ULL << 30); // 1GB
    if (builder->platformHasFastFp16()) {
        config->setFlag(nvinfer1::BuilderFlag::kFP16);
    }

    auto plan = builder->buildSerializedNetwork(*network, *config);
    std::ofstream engineFile(enginePath, std::ios::binary);
    engineFile.write((char*)plan->data(), plan->size());
    
    plan->destroy();
    config->destroy();
    network->destroy();
    builder->destroy();
    
    return true;
}

bool TrtInference::doInference(void* d_input, float* h_output, int batchSize) {
    // 1. Pre-processing (Resize/Normalize) usually done via CUDA Kernels
    // 2. Set bindings
    void* bindings[2] = { d_input, nullptr }; // Simplified
    
    // 3. Enqueue inference
    context->enqueueV2(bindings, stream, nullptr);
    
    // 4. Async copy back to host or keep in GPU for further processing
    // cudaMemcpyAsync(h_output, d_output, ...);
    
    return true;
}
