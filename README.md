# Maximum Clique Algorithm Benchmarking Suite

A comprehensive C++ project for benchmarking multiple algorithms for the Maximum Clique Problem.

## 📋 Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Algorithms Implemented](#algorithms-implemented)
- [Building the Project](#building-the-project)
- [Downloading Datasets](#downloading-datasets)
- [Running Benchmarks](#running-benchmarks)
- [Results Analysis](#results-analysis)
- [Algorithm Details](#algorithm-details)
- [Performance Characteristics](#performance-characteristics)

## 🎯 Overview

The **Maximum Clique Problem** is a classic NP-hard problem in graph theory: find the largest complete subgraph (clique) in an undirected graph. This project implements and benchmarks 9 different algorithms ranging from fast heuristics to exact exponential algorithms.

### Algorithms Categories:

1. **Fast Heuristics** (sub-second on large graphs):
   - Greedy
   - Simulated Annealing
   - Randomized Heuristic

2. **Exact Algorithms** (optimal but exponential):
   - Basic Bron-Kerbosch
   - Tomita (BK with pivoting)
   - Degeneracy Bron-Kerbosch
   - Östergård (branch-and-bound)

3. **Optimized Implementations**:
   - CPU-Optimized (bitset-based)
   - GPU-Optimized (CUDA, optional)

## 📁 Project Structure

```
maximum_clique_project/
├── include/                    # Header files
│   ├── graph.hpp              # Graph data structure
│   ├── greedy.hpp             # Greedy algorithm
│   ├── simulated_annealing.hpp
│   ├── randomized_heuristic.hpp
│   ├── bron_kerbosch.hpp      # Basic BK
│   ├── tomita.hpp             # BK with pivoting
│   ├── degeneracy_bk.hpp      # BK with degeneracy ordering
│   ├── ostergard.hpp          # Branch-and-bound with coloring
│   ├── cpu_optimized.hpp      # Bitset optimization
│   └── gpu_optimized.cuh      # CUDA version (optional)
├── src/                        # Implementation files
│   ├── graph.cpp
│   ├── greedy.cpp
│   ├── simulated_annealing.cpp
│   ├── randomized_heuristic.cpp
│   ├── bron_kerbosch.cpp
│   ├── tomita.cpp
│   ├── degeneracy_bk.cpp
│   ├── ostergard.cpp
│   ├── cpu_optimized.cpp
│   └── gpu_optimized.cu       # CUDA implementation
├── benchmark/
│   └── benchmark.cpp          # Main benchmarking program
├── datasets/                   # Place dataset files here
├── results/                    # Benchmark results output
├── CMakeLists.txt             # Build configuration
└── README.md                  # This file
```

## 🔧 Building the Project

### Prerequisites

- **C++ Compiler**: GCC 7+, Clang 6+, or MSVC 2017+
- **CMake**: Version 3.15 or higher
- **CUDA Toolkit** (optional): For GPU acceleration

### Build Instructions

```bash
# Navigate to project directory
cd maximum_clique_project

# Create build directory
mkdir build
cd build

# Configure (without CUDA)
cmake ..

# Configure (with CUDA support)
cmake -DENABLE_CUDA=ON ..

# Build
cmake --build . --config Release

# Or use make directly
make -j$(nproc)
```

### Build Output

The executable `benchmark` will be created in the `build/` directory.

## 📊 Downloading Datasets

### SNAP Stanford Network Datasets

Download datasets from the [SNAP Collection](http://snap.stanford.edu/data/):

#### Small Graphs (< 10K vertices) - All algorithms

```bash
cd datasets

# Facebook social circles (4,039 vertices, 88,234 edges)
wget http://snap.stanford.edu/data/facebook_combined.txt.gz
gunzip facebook_combined.txt.gz

# General Relativity collaboration (5,242 vertices, 14,496 edges)
wget http://snap.stanford.edu/data/ca-GrQc.txt.gz
gunzip ca-GrQc.txt.gz
```

#### Medium Graphs (10K-50K vertices) - Heuristics + Fast Exact

```bash
# High Energy Physics Theory collaboration (9,877 vertices, 25,998 edges)
wget http://snap.stanford.edu/data/ca-HepTh.txt.gz
gunzip ca-HepTh.txt.gz

# Enron email network (36,692 vertices, 183,831 edges)
wget http://snap.stanford.edu/data/email-Enron.txt.gz
gunzip email-Enron.txt.gz
```

#### Large Graphs (50K+ vertices) - Heuristics only

```bash
# Slashdot social network (77,360 vertices, 469,180 edges)
wget http://snap.stanford.edu/data/soc-Slashdot0811.txt.gz
gunzip soc-Slashdot0811.txt.gz
```

### Dataset Format

SNAP datasets use edge list format:
```
# Comment lines start with #
source_vertex target_vertex
source_vertex target_vertex
...
```

The program automatically:
- Skips comment lines
- Converts to undirected graph
- Handles arbitrary vertex IDs

## 🚀 Running Benchmarks

### Basic Usage

```bash
# Run with default datasets and output
./benchmark

# Specify custom output file
./benchmark ../results/my_results.csv
```

### Configuring Datasets

Edit `benchmark/benchmark.cpp` to add/remove datasets:

```cpp
// Add your datasets
benchmarker.add_dataset("my_graph", "../datasets/my_graph.txt");
```

### Understanding Algorithm Selection

The benchmark automatically selects algorithms based on graph size:

- **< 100 vertices**: All algorithms (including exact)
- **100-1000 vertices**: Heuristics + Degeneracy BK + CPU-Optimized
- **> 1000 vertices**: Heuristics only

### Output

Results are saved to CSV with columns:
- `dataset`: Dataset name
- `algorithm`: Algorithm name
- `time_us`: Execution time (microseconds)
- `time_ms`: Execution time (milliseconds)
- `clique_size`: Size of found clique
- `num_vertices`: Graph vertices
- `num_edges`: Graph edges
- `density`: Graph density (0 to 1)
- `valid`: Whether result is a valid clique

## 📈 Results Analysis

### Example Analysis with Python

```python
import pandas as pd
import matplotlib.pyplot as plt

# Load results
df = pd.read_csv('results/benchmark_results.csv')

# Compare clique sizes by algorithm
pivot = df.pivot(index='dataset', columns='algorithm', values='clique_size')
pivot.plot(kind='bar', figsize=(12, 6))
plt.title('Maximum Clique Size by Algorithm')
plt.ylabel('Clique Size')
plt.xlabel('Dataset')
plt.legend(bbox_to_anchor=(1.05, 1))
plt.tight_layout()
plt.savefig('clique_sizes.png')

# Compare execution times
pivot_time = df.pivot(index='dataset', columns='algorithm', values='time_ms')
pivot_time.plot(kind='bar', figsize=(12, 6), logy=True)
plt.title('Execution Time by Algorithm (log scale)')
plt.ylabel('Time (ms)')
plt.xlabel('Dataset')
plt.legend(bbox_to_anchor=(1.05, 1))
plt.tight_layout()
plt.savefig('execution_times.png')
```

## 🔍 Algorithm Details

### 1. Greedy Algorithm

**Time Complexity**: O(V² + E)  
**Space Complexity**: O(V)

Simple and fast heuristic:
1. Sort vertices by degree (descending)
2. Greedily add vertices if connected to all current clique members

**Pros**: Very fast, good approximation  
**Cons**: No optimality guarantee

---

### 2. Simulated Annealing

**Time Complexity**: O(iterations × V²)  
**Space Complexity**: O(V)

Metaheuristic with temperature-based acceptance:
- Accepts improvements always
- Accepts worse solutions with probability exp(-ΔE/T)
- Temperature decreases over time

**Parameters**:
- Initial temperature: 100.0
- Cooling rate: 0.995
- Iterations: 100,000

**Pros**: Escapes local optima, better than greedy  
**Cons**: No optimality guarantee, parameter-sensitive

---

### 3. Randomized Heuristic

**Time Complexity**: O(restarts × swaps × V²)  
**Space Complexity**: O(V)

Local search with multiple random restarts:
- Tries swapping vertices
- Multiple starting points for better exploration

**Parameters**:
- Restarts: 10
- Max swaps per restart: 1000

**Pros**: Better exploration than single-start methods  
**Cons**: Slower than greedy, no guarantee

---

### 4. Basic Bron-Kerbosch

**Time Complexity**: O(3^(V/3)) worst case  
**Space Complexity**: O(V)

Classic recursive algorithm:
- Three sets: R (current), P (candidates), X (excluded)
- Enumerates all maximal cliques
- Tracks maximum

**Pros**: Simple, guaranteed optimal  
**Cons**: Slow on dense graphs, no pruning

---

### 5. Tomita Algorithm

**Time Complexity**: O(3^(V/3)) worst case, much faster in practice  
**Space Complexity**: O(V)

Bron-Kerbosch with pivoting:
- Choose pivot to minimize recursion
- Only recurse on P \ N(pivot)

**Improvement**: 10-100× faster than basic BK

**Pros**: Optimal, significantly faster than basic BK  
**Cons**: Still exponential

---

### 6. Degeneracy Bron-Kerbosch

**Time Complexity**: O(d × 3^(d/3)) where d = degeneracy  
**Space Complexity**: O(V)

Uses degeneracy ordering:
- Process vertices in degeneracy order
- Limit P to later neighbors
- Optimal for sparse graphs

**Pros**: Excellent on sparse graphs, optimal  
**Cons**: Still exponential on dense graphs

---

### 7. Östergård Algorithm

**Time Complexity**: Exponential with pruning  
**Space Complexity**: O(V)

Branch-and-bound with coloring:
- Greedy coloring provides upper bounds
- Prune when current + bound ≤ best
- Vertex ordering by degree

**Pros**: Optimal, effective pruning  
**Cons**: Exponential, coloring overhead

---

### 8. CPU-Optimized

**Time Complexity**: Same as Tomita, but 5-10× faster  
**Space Complexity**: O(V²)

Bitset-based optimization:
- std::bitset<1024> for sets
- Bitwise AND for intersections
- __builtin_popcount for sizes

**Limitation**: Max 1024 vertices

**Pros**: Very fast, optimal  
**Cons**: Memory intensive, size limited

---

### 9. GPU-Optimized (CUDA)

**Time Complexity**: Parallel speedup  
**Space Complexity**: O(V²) on GPU

Parallel exploration of search tree:
- Thread blocks for subproblems
- Shared memory for sets
- Warp-level primitives

**Note**: Placeholder implementation in current version

**Pros**: Massive parallelism  
**Cons**: Requires CUDA, complex implementation

---

## 📊 Performance Characteristics

### Expected Performance on Different Graph Types

| Graph Type | Vertices | Density | Recommended Algorithms |
|------------|----------|---------|------------------------|
| Sparse | < 1000 | < 0.01 | Degeneracy BK, CPU-Optimized |
| Medium | 100-1000 | 0.01-0.1 | Tomita, Östergård |
| Dense | < 100 | > 0.1 | CPU-Optimized, Östergård |
| Large Sparse | > 1000 | < 0.01 | Heuristics only |
| Large Dense | > 1000 | > 0.01 | Heuristics only |

### Typical Running Times (Approximate)

| Algorithm | Small (< 100V) | Medium (100-1000V) | Large (> 1000V) |
|-----------|----------------|---------------------|-----------------|
| Greedy | < 1ms | < 10ms | < 100ms |
| Simulated Annealing | 10-100ms | 100ms-1s | 1-10s |
| Randomized Heuristic | 10-100ms | 100ms-1s | 1-10s |
| Basic BK | 1-100ms | 1s-1min | Impractical |
| Tomita | 1-50ms | 10ms-10s | Impractical |
| Degeneracy BK | 1-50ms | 10ms-10s | 10s-5min |
| Östergård | 1-100ms | 100ms-1min | Impractical |
| CPU-Optimized | < 10ms | 10ms-1s | 1s-1min |

*Times vary dramatically based on graph structure*

## 🧪 Testing Correctness

All algorithms validate results:
```cpp
bool valid = graph.is_clique(clique);
```

Checks that all vertices in returned clique are pairwise adjacent.

## 🔬 Extending the Project

### Adding a New Algorithm

1. Create header in `include/my_algorithm.hpp`
2. Implement in `src/my_algorithm.cpp`
3. Add to `CMakeLists.txt` sources
4. Add benchmark case in `benchmark/benchmark.cpp`

### Adding New Datasets

1. Place `.txt` file in `datasets/`
2. Add to benchmarker in `main()`:
   ```cpp
   benchmarker.add_dataset("name", "../datasets/file.txt");
   ```

## 📚 References

### Papers

1. **Bron-Kerbosch**: Bron, C., & Kerbosch, J. (1973). Algorithm 457: finding all cliques of an undirected graph.
2. **Tomita**: Tomita, E., Tanaka, A., & Takahashi, H. (2006). The worst-case time complexity for generating all maximal cliques.
3. **Degeneracy Ordering**: Eppstein, D., Löffler, M., & Strash, D. (2010). Listing all maximal cliques in sparse graphs in near-optimal time.
4. **Östergård**: Östergård, P. R. (2002). A fast algorithm for the maximum clique problem.

### Datasets

- **SNAP**: Stanford Large Network Dataset Collection - http://snap.stanford.edu/data/

## 📝 License

This project is for educational purposes (Algorithms course project).

## 👥 Authors

Algorithms Course Project

## 🐛 Known Issues

- GPU implementation is placeholder only
- CPU-Optimized limited to 1024 vertices
- Exact algorithms may timeout on dense graphs > 100 vertices

## 🚀 Future Improvements

1. Implement full CUDA GPU version
2. Add parallel CPU version with OpenMP
3. Implement more heuristics (Genetic Algorithm, Ant Colony)
4. Add graph generators for synthetic benchmarks
5. Implement approximation algorithms with guarantees
6. Add visualization of found cliques
7. Support for dynamic bitsets (unlimited vertices)

---

**Happy benchmarking! 🎉**
