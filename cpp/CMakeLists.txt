cmake_minimum_required(VERSION 3.20)
project(VisionStreamX100 LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Dependencies
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

# TensorRT
set(TENSORRT_ROOT "/usr/lib/x86_64-linux-gnu") # Default Ubuntu path
include_directories(${TENSORRT_ROOT}/include)
link_directories(${TENSORRT_ROOT}/lib)

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
    nvinfer 
    nvcuvid 
    pthread
)

# Optimization Flags
target_compile_options(vision_stream_x100 PRIVATE -O3 -march=native)
