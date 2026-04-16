cmake_minimum_required(VERSION 3.20)
project(VisionStreamX100 LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Dependencies
find_package(CUDA REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

# TensorRT & nvJPEG Search
include_directories(/usr/include/x86_64-linux-gnu)
include_directories(/usr/local/cuda/include)
link_directories(/usr/lib/x86_64-linux-gnu)
link_directories(/usr/local/cuda/lib64)

# Source Files
add_executable(vision_stream_x100 
    main.cpp 
    GpuDecoder.cpp 
    TrtInference.cpp
    Pipeline.cpp
)

# Link Libraries
target_link_libraries(vision_stream_x100 
    PRIVATE 
    ${CUDA_LIBRARIES}
    ${CURL_LIBRARIES}
    ${OpenCV_LIBS}
    nvinfer
    nvonnxparser
    nvcuvid
    nvjpeg
    cuda
    cudart
    dl
    pthread
)

# Optimization Flags
target_compile_options(vision_stream_x100 PRIVATE -O3 -march=native)
