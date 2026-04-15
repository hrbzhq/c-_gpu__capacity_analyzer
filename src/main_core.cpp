#include "vision/Pipeline.hpp"
#include <iostream>
#include <csignal>

std::atomic<bool> g_keepRunning{true};

void signalHandler(int signum) {
    g_keepRunning = false;
}

int main() {
    signal(SIGINT, signalHandler);

    std::cout << "VisionStream X100 Core Initialized." << std::endl;
    std::cout << "Hardware: Dual RTX 6000 Ada | 96GB VRAM | 512GB RAM" << std::endl;

    vision::Pipeline pipeline(100);
    pipeline.start();

    while (g_keepRunning) {
        auto metrics = pipeline.getMetrics();
        // In real app, push these to a shared memory or socket for the API
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    pipeline.stop();
    std::cout << "System Shutdown Cleanly." << std::endl;
    return 0;
}
