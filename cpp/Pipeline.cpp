#include "Pipeline.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <curl/curl.h>
#include <sstream>

// CURL write callback for HTTP response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Simple JSON parser to extract URL values (assumes {"url": "rtsp://..."} format)
static std::vector<std::string> parseCameraUrls(const std::string& json) {
    std::vector<std::string> urls;
    size_t pos = 0;
    
    // Look for "url": patterns in JSON
    while ((pos = json.find("\"url\"", pos)) != std::string::npos) {
        // Find the value after "url":
        pos = json.find(":", pos);
        if (pos == std::string::npos) break;
        
        // Find opening quote
        pos = json.find("\"", pos + 1);
        if (pos == std::string::npos) break;
        
        size_t endPos = json.find("\"", pos + 1);
        if (endPos == std::string::npos) break;
        
        std::string url = json.substr(pos + 1, endPos - pos - 1);
        if (url.find("rtsp://") == 0) {
            urls.push_back(url);
        }
        pos = endPos + 1;
    }
    
    return urls;
}

Pipeline::Pipeline(const std::vector<std::string>& enginePaths, 
                   const std::string& cameraApiUrl) 
    : numStreams(0), cameraApiUrl(cameraApiUrl), enginePaths(enginePaths) {
    
    // Initialize 4 inference engines (model array)
    // GPU 0 gets engines 0,1; GPU 1 gets engines 2,3
    for (size_t i = 0; i < enginePaths.size() && i < 4; ++i) {
        int gpuId = (i < 2) ? 0 : 1;
        inferenceEngines.push_back(std::make_unique<TrtInference>(gpuId, enginePaths[i]));
        std::cout << "Loaded engine " << i << " on GPU " << gpuId << ": " << enginePaths[i] << std::endl;
    }
}

bool Pipeline::fetchCameraUrls() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }
    
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, cameraApiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "HTTP request failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);
    
    if (httpCode != 200) {
        std::cerr << "HTTP error code: " << httpCode << std::endl;
        return false;
    }
    
    cameraUrls = parseCameraUrls(response);
    numStreams = cameraUrls.size();
    
    std::cout << "Fetched " << numStreams << " camera URLs from API" << std::endl;
    
    // Initialize decoders for each camera URL
    for (int i = 0; i < numStreams; ++i) {
        int gpuId = (i < numStreams / 2) ? 0 : 1;
        decoders.push_back(std::make_unique<GpuDecoder>(gpuId, cameraUrls[i]));
    }
    
    return !cameraUrls.empty();
}

Pipeline::~Pipeline() {
    stop();
}

void Pipeline::start() {
    if (cameraUrls.empty()) {
        std::cerr << "No camera URLs available. Call fetchCameraUrls() first." << std::endl;
        return;
    }
    
    running = true;
    for (int i = 0; i < numStreams; ++i) {
        int gpuId = (i < numStreams / 2) ? 0 : 1;
        // Round-robin assign engine: GPU 0 uses engines 0,1; GPU 1 uses engines 2,3
        int engineIdx = (i % 2) + (gpuId * 2);
        workerThreads.push_back(std::make_unique<std::thread>(&Pipeline::streamWorker, this, i, gpuId, engineIdx));
    }
}

void Pipeline::stop() {
    running = false;
    for (auto& thread : workerThreads) {
        if (thread->joinable()) thread->join();
    }
    workerThreads.clear();
}

void Pipeline::streamWorker(int streamId, int gpuId, int engineIdx) {
    auto& decoder = decoders[streamId];
    // Use provided engineIdx or calculate from stream ID
    int actualEngineIdx = engineIdx;
    if (actualEngineIdx >= static_cast<int>(inferenceEngines.size())) {
        actualEngineIdx = 0;
    }
    auto& engine = inferenceEngines[actualEngineIdx];

    if (!decoder->initialize()) {
        std::cerr << "Failed to initialize decoder for stream " << streamId << std::endl;
        return;
    }

    while (running) {
        void* d_frame = nullptr;
        size_t pitch = 0;

        // 1. Hardware-accelerated Grab (NVDEC)
        if (decoder->grabFrame(&d_frame, pitch)) {
            
            // 2. TensorRT Inference using 4-model array
            float output[1000]; // Example output buffer
            engine->doInference(d_frame, output, 1);
            
            // 3. Post-processing or Metadata Push
            // ...
        }

        // Throttle to target FPS if necessary (40ms = 25 FPS target)
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

Pipeline::Metrics Pipeline::getMetrics() {
    // In a real app, these would be calculated from atomic counters
    return {
        2500.0f, // Total FPS (25 FPS * 100 streams)
        15.5f,   // Avg Latency
        {65.0f, 62.0f}, // GPU Util
        {18.5f, 17.8f}  // VRAM Usage
    };
}

int Pipeline::calculateMaxCapacity() {
    // Capacity calculation for Dual RTX 4090 (User Spec: 96GB Total VRAM)
    // Scenario: Frame Sampling (3 FPS per stream instead of 25 FPS)
    const float targetFps = 3.0f;
    
    // 1. VRAM Constraint: 48GB per GPU (96GB Total)
    // Each stream still needs a base VRAM footprint for decoder context and buffers
    // even at low FPS. ~100MB per stream.
    const float vramPerStreamGb = 0.10f; 
    const float totalVramGb = 96.0f; 
    int vramLimit = static_cast<int>(totalVramGb / vramPerStreamGb);

    // 2. NVDEC Constraint: 2 units per GPU, ~1000 FPS total per GPU.
    // At 3 FPS: (1000 * 2) / 3 = 666 streams.
    int nvdecLimit = static_cast<int>((1000 * 2) / targetFps); 

    // 3. Inference Constraint: RTX 4090 @ YOLOv8n
    // Total throughput is ~833 FPS per GPU.
    // At 3 FPS: (833 * 2) / 3 = 555 streams.
    int inferenceLimit = static_cast<int>((833 * 2) / targetFps);

    // 4. CPU/Network Constraint: Handling 500+ RTSP streams 
    // requires significant CPU for demuxing and network interrupts.
    int cpuLimit = 500; 

    // With 3 FPS sampling, the bottleneck shifts from NVDEC to Inference/CPU.
    int minVal = std::min(vramLimit, nvdecLimit);
    minVal = std::min(minVal, inferenceLimit);
    minVal = std::min(minVal, cpuLimit);
    return minVal;
}
