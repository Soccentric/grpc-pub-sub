#!/bin/bash
# Script to install gRPC and Protobuf dependencies from source

set -e  # Exit on error

echo "Installing gRPC and Protobuf from source"

# Install necessary build tools
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    autoconf \
    libtool \
    pkg-config \
    cmake \
    git \
    curl

# Clone the gRPC repository
if [ ! -d "grpc" ]; then
    git clone --recurse-submodules -b v1.50.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
fi

# Build and install gRPC and its dependencies (including Protobuf)
cd grpc
mkdir -p cmake/build
cd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ../..
make -j$(nproc)
sudo make install
sudo ldconfig

echo "gRPC and Protobuf installed successfully"

# Return to the project directory
cd ../../../

# Build the project
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo "Project built successfully. Executables are in the 'build' directory."
echo "Run the subscriber with: ./build/subscriber"
echo "Run the publisher with: ./build/publisher"