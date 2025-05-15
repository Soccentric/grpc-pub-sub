#!/bin/bash
# Script to install gRPC and Protobuf dependencies from packages

set -e  # Exit on error

echo "Installing gRPC and Protobuf dependencies"

# Update package lists
sudo apt-get update

# Install necessary dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libprotobuf-dev \
    protobuf-compiler \
    libgrpc++-dev \
    protobuf-compiler-grpc

echo "Dependencies installed successfully"

# Build the project
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo "Project built successfully. Executables are in the 'build' directory."
echo "Run the subscriber with: ./build/subscriber"
echo "Run the publisher with: ./build/publisher"