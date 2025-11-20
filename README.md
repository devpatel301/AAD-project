# Maximum Clique Algorithm Benchmarking Suite

![C++](https://img.shields.io/badge/Language-C++17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.8+-yellow.svg)
![CMake](https://img.shields.io/badge/Build-CMake-green.svg)
![License](https://img.shields.io/badge/License-MIT-orange.svg)

A high-performance C++ framework implementing **11 different algorithms** for solving the Maximum Clique Problem, ranging from fast heuristics to exact branch-and-bound methods. This suite includes extensive benchmarking capabilities across **22 diverse graph datasets** in standardized DIMACS format.

---

## Table of Contents
- [Overview](#overview)
- [Algorithms Implemented](#algorithms-implemented)
- [Datasets](#datasets)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Performance Benchmarks](#performance-benchmarks)
- [Algorithm Selection Guide](#algorithm-selection-guide)
- [References](#references)

---

## Overview

The **Maximum Clique Problem** is a classic NP-hard problem in graph theory: find the largest complete subgraph (clique) in an undirected graph.

This repository serves as both a learning tool and a performance benchmark suite. It features:
* **Exact Algorithms:** Guarantee optimal solutions using Branch-and-Bound and bitset optimizations.
* **Heuristics:** Provide near-optimal solutions in polynomial time for massive graphs.
* **Optimizations:** Implementations include bit-parallelism, vertex ordering, graph coloring bounds, and degeneracy ordering.

---

## Algorithms Implemented

### Heuristic Algorithms (Approximate)
*These algorithms find **maximal** cliques quickly but do not guarantee the **maximum** (largest) clique.*

| Algorithm | Time Complexity | Description |
|-----------|----------------|-------------|
| **Greedy** | $O(V^2 + E)$ | Iteratively adds highest-degree vertices to the clique. Very fast but often produces smaller cliques. |
| **Randomized Heuristic** | $O(R \times S \times V^2)$ | Local search with random restarts to escape local optima. Modified with random initialization and swap operations. |
| **Simulated Annealing** | $O(I \times V^2)$ | Probabilistic metaheuristic allowing worse moves to escape local optima using an adaptive cooling schedule. |

### Exact Algorithms (Optimal)
*These algorithms guarantee finding the **maximum** clique but generally have exponential time complexity.*

| Algorithm | Complexity (Worst) | Key Optimizations & Modifications |
|-----------|-------------------|-----------------------------------|
| **Bron-Kerbosch** | $O(3^{n/3})$ | Basic recursive backtracking. **Modified with:** Pruning based on $\|R\| + \|P\| \le \text{best\_size}$. |
| **Tomita** | $O(3^{n/3})$ | BK with pivoting. **Modified with:** Pivot selection maximizing $\|P \cap N(\text{pivot})\|$ to minimize recursive branches. |
| **Degeneracy BK** | $O(d \cdot 3^{d/3})$ | Uses degeneracy ordering. **Modified with:** Optimal for sparse graphs; finds each maximal clique exactly once. |
| **Östergård** | $O(3^{n/3})$ | Branch-and-bound. **Modified with:** Degree-based vertex ordering and early termination when upper bound $\le$ current best. |
| **BBMC** | $O(3^{n/3})$ | **State-of-the-art.** Bitset-based B&B. **Modified with:** $O(1)$ intersections, greedy coloring bounds, and min-width ordering. |
| **CPU Optimized** | $O(3^{n/3})$ | Highly optimized Tomita variant using `std::bitset` and cache-friendly memory layout (Limited to 1024 vertices). |
| **GPU Optimized** | $O(3^{n/3})$ | CUDA-accelerated parallel search using thread blocks and warp-level primitives (Placeholder/Experimental). |

---

## Datasets

All datasets are standardized in **DIMACS format**.

### Real-World Networks (SNAP)
| Dataset | Vertices | Edges | Density | Description |
|---------|----------|-------|---------|-------------|
| **facebook_combined** | 4,039 | 88,234 | 1.08% | Social circles from Facebook. |
| **twitter_combined** | 81,306 | 2,420,766 | 0.07% | Twitter follower relationships. |
| **email-Eu-core** | 1,005 | 16,706 | 3.31% | European research institution emails. |
| **ca-GrQc** | 26,197 | 14,496 | <0.01% | General Relativity collaboration network. |

### Synthetic & Benchmark Graphs
| Category | Count | Description |
|----------|:-----:|-------------|
| **R-MAT (Synthetic)** | 6 | Recursive Matrix generated graphs. Includes Erdős-Rényi and Power-Law distributions (200-500 vertices). |
| **3-SAT Derived** | 2 | Graphs generated from 3-SAT problems based on cryptographic hash functions. |
| **DIMACS Challenge** | 12 | Classic hard instances including `brock`, `keller`, and `p_hat` series. |

---

## Project Structure

```text
maximum_clique_project/
├── benchmark/                  # Main benchmarking executable source
│   └── benchmark.cpp
├── datasets/                   # Graph datasets (DIMACS format)
│   ├── benchmark/              # Standard DIMACS/BHOSLIB instances
│   ├── real_world/             # SNAP social networks
│   └── synthetic/              # R-MAT and SAT generated graphs
├── include/                    # Header files
│   ├── graph.hpp               # Graph data structure
│   ├── bbmc.hpp                # Bitset Branch-and-Bound
│   ├── tomita.hpp              # Pivoting BK
│   └── ... (other headers)
├── src/                        # Algorithm implementations
│   ├── graph.cpp
│   ├── greedy.cpp
│   ├── bbmc.cpp
│   └── ... (other sources)
├── scripts/                    # Python utility scripts
│   ├── generate_graphs.py
│   └── download_datasets.py
├── comprehensive_benchmark.ipynb  # Jupyter notebook for analysis
├── CMakeLists.txt              # Build configuration
└── README.md