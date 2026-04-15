# Use NVIDIA CUDA base image with development tools
FROM nvidia/cuda:12.2.0-devel-ubuntu22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV NODE_VERSION=20.x

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    pkg-config \
    libopencv-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libgstrtspserver-1.0-dev \
    && rm -rf /var/lib/apt/lists/*

# Install TensorRT (Note: In a real scenario, you might need to add NVIDIA's repo or local deb)
# This assumes the base image or standard repos have the necessary headers
RUN apt-get update && apt-get install -y \
    libnvinfer-dev \
    libnvonnxparser-dev \
    libnvparsers-dev \
    libnvinfer-plugin-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_$NODE_VERSION | bash - \
    && apt-get install -y nodejs

# Set working directory
WORKDIR /app

# Copy package.json and install Node dependencies
COPY package*.json ./
RUN npm install

# Copy the rest of the application
COPY . .

# Build C++ Backend
RUN mkdir -p cpp/build && cd cpp/build && \
    cmake .. && \
    make -j$(nproc)

# Build Frontend
RUN npm run build

# Expose the application port
EXPOSE 3000

# Start the server
# Note: In production, we run the compiled server. In dev, we use tsx.
CMD ["npm", "run", "dev"]
