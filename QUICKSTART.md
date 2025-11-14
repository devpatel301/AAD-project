# Quick Start Guide

## 🚀 Get Started in 5 Minutes

### Step 1: Build the Project

```bash
cd maximum_clique_project
chmod +x build.sh
./build.sh
```

This will compile all algorithms and create the `benchmark` executable.

### Step 2: Download a Test Dataset

```bash
cd datasets
wget http://snap.stanford.edu/data/facebook_combined.txt.gz
gunzip facebook_combined.txt.gz
cd ..
```

Or use the included test dataset:
```bash
# Already included: datasets/test_small.txt
```

### Step 3: Run the Benchmark

```bash
cd build
./benchmark
```

### Step 4: View Results

```bash
cat ../results/benchmark_results.csv
```

---

## 📊 Quick Results Analysis

### Using Python

```bash
cd results
pip install pandas matplotlib seaborn  # If not already installed
python analyze_results.py benchmark_results.csv
```

This generates:
- `clique_sizes.png` - Comparison of clique sizes
- `execution_times.png` - Time comparison
- `quality_vs_speed.png` - Tradeoff analysis
- `speedup_comparison.png` - Relative speedups
- `results_table.tex` - LaTeX table for reports

### Using Excel/Google Sheets

Simply open `results/benchmark_results.csv` in your spreadsheet application.

---

## 🎯 Testing Individual Algorithms

### Quick Test Program

Create `test/quick_test.cpp`:

```cpp
#include "graph.hpp"
#include "greedy.hpp"
#include "tomita.hpp"
#include <iostream>
#include <chrono>

int main() {
    // Load graph
    Graph g = Graph::load_from_snap("../datasets/test_small.txt");
    
    std::cout << "Graph: " << g.num_vertices() << " vertices, "
              << g.num_edges() << " edges\n\n";
    
    // Test Greedy
    auto start = std::chrono::high_resolution_clock::now();
    auto clique = GreedyClique::find_clique(g);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Greedy Algorithm:\n";
    std::cout << "  Clique size: " << clique.size() << "\n";
    std::cout << "  Time: " << duration.count() << " μs\n";
    std::cout << "  Valid: " << (g.is_clique(clique) ? "Yes" : "No") << "\n\n";
    
    // Test Tomita
    start = std::chrono::high_resolution_clock::now();
    TomitaAlgorithm tomita;
    clique = tomita.find_maximum_clique(g);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Tomita Algorithm (Exact):\n";
    std::cout << "  Clique size: " << clique.size() << "\n";
    std::cout << "  Time: " << duration.count() << " μs\n";
    std::cout << "  Valid: " << (g.is_clique(clique) ? "Yes" : "No") << "\n";
    
    return 0;
}
```

---

## 🔧 Common Tasks

### Adding a New Dataset

1. Place `.txt` file in `datasets/`
2. Edit `benchmark/benchmark.cpp`:
   ```cpp
   benchmarker.add_dataset("my_graph", "../datasets/my_graph.txt");
   ```
3. Rebuild and run

### Testing on Custom Graph

Create edge list file:
```
# my_graph.txt
0 1
0 2
1 2
2 3
```

Run:
```bash
./benchmark
```

### Adjusting Algorithm Parameters

Edit the benchmark.cpp file to change parameters:

```cpp
// Simulated Annealing: temperature, cooling, iterations
SimulatedAnnealing sa(200.0, 0.99, 200000, 42);

// Randomized Heuristic: restarts, swaps
RandomizedHeuristic rh(20, 2000, 42);
```

---

## 📈 Understanding Output

### Console Output

```
Loaded graph: 4039 vertices, 88234 edges

Benchmarking dataset: facebook
  Vertices: 4039
  Edges: 88234
  Density: 0.010837
  Degeneracy: 115

  Running Greedy... Done! Clique size: 17, Time: 2.45 ms [VALID]
  Running Tomita... Done! Clique size: 17, Time: 145.32 ms [VALID]
  ...
```

### CSV Output

Columns:
- `dataset`: Dataset name
- `algorithm`: Algorithm used
- `time_us`: Execution time in microseconds
- `time_ms`: Execution time in milliseconds
- `clique_size`: Size of found clique
- `num_vertices`: Number of vertices
- `num_edges`: Number of edges
- `density`: Graph density (0-1)
- `valid`: Whether result is valid clique

---

## 🐛 Troubleshooting

### Build Errors

**Problem**: `cmake not found`
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# MacOS
brew install cmake
```

**Problem**: `g++ not found`
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# MacOS
xcode-select --install
```

### Runtime Errors

**Problem**: `Cannot open file: ../datasets/facebook_combined.txt`
- Check file exists: `ls datasets/`
- Download dataset (see DATASETS.md)
- Check path in benchmark.cpp

**Problem**: Algorithm takes too long
- Use smaller dataset for exact algorithms
- Limit to heuristics for large graphs
- See performance table in README.md

### Analysis Errors

**Problem**: Python packages missing
```bash
pip install pandas matplotlib seaborn
```

**Problem**: No display for plots
```python
# Add to analyze_results.py
import matplotlib
matplotlib.use('Agg')  # For non-GUI environments
```

---

## 📝 Quick Reference

### Build Commands

```bash
./build.sh              # Standard build
./build.sh --debug      # Debug build
./build.sh --cuda       # With CUDA
./build.sh --clean      # Clean build
./build.sh --run        # Build and run
```

### CMake Commands

```bash
mkdir build && cd build
cmake ..                        # Configure
cmake --build .                 # Build
cmake --build . --config Release  # Release build
```

### File Locations

```
build/benchmark              # Main executable
results/benchmark_results.csv  # Output results
results/*.png                # Generated plots
datasets/*.txt               # Input graphs
```

---

## 🎓 Learning More

### Understanding Algorithms

1. **Greedy**: Start with README.md algorithm details
2. **Exact**: Read paper references in README.md
3. **Implementation**: Check source code with detailed comments

### Modifying Algorithms

All algorithms follow this pattern:

```cpp
class MyAlgorithm {
public:
    std::vector<int> find_maximum_clique(const Graph& g);
private:
    std::vector<int> max_clique;
    // Helper methods
};
```

### Adding Your Own Algorithm

1. Create `include/my_algo.hpp` and `src/my_algo.cpp`
2. Implement `find_maximum_clique(const Graph& g)`
3. Add to CMakeLists.txt
4. Add benchmark case in benchmark.cpp

---

## 💡 Tips for Best Results

1. **Start Small**: Use test datasets first
2. **Choose Wisely**: Match algorithm to graph size (see README.md table)
3. **Multiple Runs**: Run 3-5 times for stable timing
4. **Verify Results**: Always check the `valid` column
5. **Compare Fairly**: Same hardware, same build settings

---

## 📞 Need Help?

1. Check README.md for detailed information
2. Review DATASETS.md for dataset issues
3. Check source code comments
4. Verify build configuration

---

**Happy benchmarking! 🎉**
