#!/bin/bash

# Dataset downloader for Maximum Clique Benchmarking Project

echo "=========================================="
echo "SNAP Dataset Downloader"
echo "=========================================="
echo

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check if we're in the datasets directory
if [ ! -f "DATASETS.md" ]; then
    echo "Please run this script from the datasets/ directory"
    exit 1
fi

# Function to download and extract
download_dataset() {
    local name=$1
    local url=$2
    local filename=$3
    
    echo -e "${YELLOW}Downloading $name...${NC}"
    
    if [ -f "$filename" ]; then
        echo "  File already exists, skipping"
        return
    fi
    
    wget -q --show-progress "$url" || {
        echo "  Failed to download"
        return 1
    }
    
    if [[ $url == *.gz ]]; then
        echo "  Extracting..."
        gunzip -f "${filename}.gz"
    fi
    
    echo -e "${GREEN}  ✓ Complete${NC}"
}

# Small datasets
echo "=== Small Datasets (< 10K vertices) ==="
echo

download_dataset "Facebook Social Circles" \
    "http://snap.stanford.edu/data/facebook_combined.txt.gz" \
    "facebook_combined.txt"

download_dataset "ca-GrQc (General Relativity)" \
    "http://snap.stanford.edu/data/ca-GrQc.txt.gz" \
    "ca-GrQc.txt"

echo
read -p "Download medium datasets (10K-50K vertices)? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "=== Medium Datasets ==="
    echo
    
    download_dataset "ca-HepTh (High Energy Physics)" \
        "http://snap.stanford.edu/data/ca-HepTh.txt.gz" \
        "ca-HepTh.txt"
    
    download_dataset "Enron Email Network" \
        "http://snap.stanford.edu/data/email-Enron.txt.gz" \
        "email-Enron.txt"
    
    download_dataset "ca-HepPh (Physics Phenomenology)" \
        "http://snap.stanford.edu/data/ca-HepPh.txt.gz" \
        "ca-HepPh.txt"
fi

echo
read -p "Download large datasets (50K+ vertices)? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "=== Large Datasets (Heuristics only!) ==="
    echo
    
    download_dataset "Slashdot Social Network" \
        "http://snap.stanford.edu/data/soc-Slashdot0811.txt.gz" \
        "soc-Slashdot0811.txt"
    
    download_dataset "Epinions Social Network" \
        "http://snap.stanford.edu/data/soc-Epinions1.txt.gz" \
        "soc-Epinions1.txt"
fi

echo
echo "=========================================="
echo "Download Complete!"
echo "=========================================="
echo
echo "Downloaded datasets:"
ls -lh *.txt 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo
echo "Next steps:"
echo "  1. cd .."
echo "  2. ./build.sh"
echo "  3. cd build && ./benchmark"
echo
