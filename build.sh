#!/bin/bash

# Build script for Maximum Clique Benchmarking Project

set -e  # Exit on error

echo "=========================================="
echo "Maximum Clique Benchmark - Build Script"
echo "=========================================="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking prerequisites..."

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake not found. Please install CMake 3.15 or later.${NC}"
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: No C++ compiler found. Please install g++ or clang++.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Prerequisites OK${NC}"
echo

# Parse arguments
BUILD_TYPE="Release"
ENABLE_CUDA="OFF"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --cuda)
            ENABLE_CUDA="ON"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        --help)
            echo "Usage: ./build.sh [options]"
            echo
            echo "Options:"
            echo "  --debug    Build in Debug mode (default: Release)"
            echo "  --cuda     Enable CUDA support"
            echo "  --clean    Clean build directory before building"
            echo "  --run      Run benchmark after successful build"
            echo "  --help     Show this help message"
            exit 0
            ;;
        *)
            echo -e "${YELLOW}Warning: Unknown option $1${NC}"
            shift
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo -e "${GREEN}✓ Clean complete${NC}"
    echo
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo "Configuring CMake..."
echo "  Build Type: $BUILD_TYPE"
echo "  CUDA Support: $ENABLE_CUDA"
echo

cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DENABLE_CUDA=$ENABLE_CUDA

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Configuration failed${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Configuration complete${NC}"
echo

# Build
echo "Building project..."
cmake --build . --config $BUILD_TYPE -- -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

echo
echo -e "${GREEN}✓ Build successful!${NC}"
echo

# Display binary location
echo "Executable location: $(pwd)/benchmark"
echo

# Run if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo "=========================================="
    echo "Running benchmark..."
    echo "=========================================="
    echo
    ./benchmark
fi

# Show next steps
echo "=========================================="
echo "Next Steps:"
echo "=========================================="
echo
echo "1. Download datasets (if not already done):"
echo "   cd ../datasets"
echo "   wget http://snap.stanford.edu/data/facebook_combined.txt.gz"
echo "   gunzip facebook_combined.txt.gz"
echo
echo "2. Run benchmark:"
echo "   cd build"
echo "   ./benchmark"
echo
echo "3. View results:"
echo "   cat ../results/benchmark_results.csv"
echo
echo "=========================================="
