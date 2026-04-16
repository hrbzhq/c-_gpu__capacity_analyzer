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
    if (context) delete context;
    if (engine) delete engine;
    if (runtime) delete runtime;
    if (stream) cudaStreamDestroy(stream);
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
    // TensorRT 10: Explicit batch is now the default, kEXPLICIT_BATCH is deprecated
    auto network = builder->createNetworkV2(0);
    auto config = builder->createBuilderConfig();
    auto parser = nvonnxparser::createParser(*network, gLogger);

    if (!parser->parseFromFile(onnxPath.c_str(), static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
        std::cerr << "Failed to parse ONNX file" << std::endl;
        return false;
    }

    config->setMemoryPoolLimit(nvinfer1::MemoryPoolType::kWORKSPACE, 1ULL << 30); // 1GB
    if (builder->platformHasFastFp16()) {
        config->setFlag(nvinfer1::BuilderFlag::kFP16);
    }

    auto plan = builder->buildSerializedNetwork(*network, *config);
    std::ofstream engineFile(enginePath, std::ios::binary);
    engineFile.write((char*)plan->data(), plan->size());
    
    delete plan;
    delete config;
    delete network;
    delete builder;
    delete parser;
    
    return true;
}

bool TrtInference::doInference(void* d_input, float* h_output, int batchSize) {
    if (!d_input || !context) {
        return false;
    }
    
    // 1. Pre-processing (Resize/Normalize) usually done via CUDA Kernels
    
    // 2. Set tensor addresses for enqueueV3 (TensorRT 10.x API)
    // Get tensor names and set addresses
    const char* inputName = engine->getIOTensorName(0);
    const char* outputName = engine->getIOTensorName(1);
    
    context->setTensorAddress(inputName, d_input);
    // For output, we would need a GPU buffer. Using nullptr for now.
    context->setTensorAddress(outputName, nullptr);
    
    // 3. Enqueue inference using enqueueV3 (TensorRT 10.x)
    bool status = context->enqueueV3(stream);
    
    // 4. Async copy back to host or keep in GPU for further processing
    // cudaMemcpyAsync(h_output, d_output, ...);
    
    return status;
}
