# VisionStream X100 Compilation Guide

This system is designed for **Ubuntu 24.04** with **Dual RTX 4090** GPUs.

## Prerequisites

1. **NVIDIA Driver**: 550+
2. **CUDA Toolkit**: 12.4+
3. **TensorRT**: 10.0+
4. **NVIDIA Video Codec SDK**: 12.1+
5. **FFmpeg**: With `h264_cuvid` and `hevc_cuvid` support.

## Dependencies Installation

```bash
sudo apt update
sudo apt install -y build-essential cmake libffmpeg-dev libnvinfer-dev
```

## Compilation

Use the provided `CMakeLists.txt` (to be created) or compile manually:

```bash
g++ -O3 main.cpp GpuDecoder.cpp TrtInference.cpp \
    -I/usr/local/cuda/include \
    -L/usr/local/cuda/lib64 \
    -lcudart -lnvinfer -lnvcuvid \
    -lpthread -o vision_stream_x100
```

## Performance Tuning

1. **GPU Persistence Mode**: `sudo nvidia-smi -pm 1`
2. **GPU Performance State**: `sudo nvidia-smi -lgc 2100` (Lock clocks for stability)
3. **HugePages**: Enable for large VRAM allocations.
