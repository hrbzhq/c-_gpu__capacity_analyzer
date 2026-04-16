#include "Pipeline.hpp"
#include "httplib.h" // Header-only library
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <nlohmann/json.hpp> // Correct path for Ubuntu nlohmann-json3-dev package

using json = nlohmann::json;

int main() {
    std::cout << "Starting VisionStream X100 High-Performance Video System..." << std::endl;
    
    // 1. Initialize Pipeline
    std::vector<std::string> enginePaths = {
        "c-_gpu__capacity_analyzer-main/yolo_models/20251027_205844.engine",
        "c-_gpu__capacity_analyzer-main/yolo_models/20251104_172108.engine",
        "c-_gpu__capacity_analyzer-main/yolo_models/20251113_160034.engine",
        "c-_gpu__capacity_analyzer-main/yolo_models/s20251204_162540.engine"
    };
    const std::string cameraApiUrl = "http://192.168.8.191:9061/api/cameras/list";
    Pipeline pipeline(enginePaths, cameraApiUrl);

    if (!pipeline.fetchCameraUrls()) {
        std::cerr << "Failed to fetch camera URLs. Exiting." << std::endl;
        return 1;
    }
    pipeline.start();

    // 2. Setup HTTP API Server (Port 8080)
    httplib::Server svr;

    svr.Get("/api/metrics", [&](const httplib::Request&, httplib::Response& res) {
        auto m = pipeline.getMetrics();
        json j;
        j["numStreams"] = pipeline.getNumStreams();
        j["totalFps"] = m.totalFps;
        j["avgLatency"] = m.avgLatencyMs;
        j["gpuUtil"] = {m.gpuUtilization[0], m.gpuUtilization[1]};
        j["vramUsage"] = {m.vramUsageGb[0], m.vramUsageGb[1]};
        
        res.set_content(j.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // MJPEG Stream Endpoint for a specific camera
    svr.Get(R"(/api/stream/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        int camId = std::stoi(req.matches[1]);
        
        res.set_header("Content-Type", "multipart/x-mixed-replace; boundary=frame");
        res.set_chunked_content_provider("multipart/x-mixed-replace; boundary=frame",
            [camId, &pipeline](size_t offset, httplib::DataSink &sink) {
                if (!sink.is_writable()) return false;
                
                // Get the latest encoded JPEG from the pipeline for this camera
                std::vector<uchar> jpegData;
                if (pipeline.getLatestFrame(camId, jpegData)) {
                    std::string header = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + 
                                         std::to_string(jpegData.size()) + "\r\n\r\n";
                    sink.write(header.data(), header.size());
                    sink.write((const char*)jpegData.data(), jpegData.size());
                    sink.write("\r\n", 2);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ~10 FPS preview
                return true;
            }
        );
    });

    // 3. Run HTTP Server in a separate thread
    std::thread apiThread([&]() {
        std::cout << "HTTP API Server listening on http://0.0.0.0:8080" << std::endl;
        svr.listen("0.0.0.0", 8080);
    });

    // 4. Main monitoring loop (Console output)
    while (true) {
        auto metrics = pipeline.getMetrics();
        printf("\r[SYSTEM] Streams: %d | Total FPS: %.2f | GPU0: %.1f%% | GPU1: %.1f%%", 
               pipeline.getNumStreams(), metrics.totalFps, 
               metrics.gpuUtilization[0], metrics.gpuUtilization[1]);
        fflush(stdout);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
