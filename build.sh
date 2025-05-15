#!/bin/bash
# Build script for gRPC Publish-Subscribe System

set -e  # Exit on any error

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Generate build files
echo "Generating build files..."
cmake ..

# Build the project
echo "Building project..."
cmake --build .

echo "Build completed successfully."
echo ""
echo "To run the subscriber (server):"
echo "  ./build/subscriber [server_address]"
echo ""
echo "To run the publisher (client):"
echo "  ./build/publisher [server_address] [topic] [message]"
