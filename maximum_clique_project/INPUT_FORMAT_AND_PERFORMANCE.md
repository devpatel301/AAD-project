# Dataset Input Format and Algorithm Performance Guide

## 📋 Input Format

### Current Supported Format: Edge List

The project uses a **simple edge list format** (compatible with SNAP datasets):

```
# Lines starting with # are comments (optional)
# Format: source_vertex target_vertex (space or tab separated)

0 1
0 2
1 2
2 3
```

### Format Specifications

| Property | Details |
|----------|---------|
| **Separator** | Space or tab |
| **Comments** | Lines starting with `#` |
| **Vertex IDs** | Any non-negative integers (0-indexed) |
| **Direction** | Automatically converted to undirected |
| **Self-loops** | Automatically ignored |
| **Duplicates** | Automatically handled |

### Example: Small Graph

```
# Triangle graph (maximum clique = 3)
0 1
1 2
0 2
```

This creates an undirected graph:
- 3 vertices (0, 1, 2)
- 3 edges
- Forms a complete triangle (clique of size 3)

### Example: Graph with Comments

```
# Test graph for maximum clique
# Contains a clique of size 4

# Clique edges
0 1
0 2
0 3
1 2
1 3
2 3

# Additional vertices
0 4
1 4
```

---

## ⚠️ Can I Run Bron-Kerbosch on Large/Dense Graphs?

### Short Answer: **IT DEPENDS** 

The performance depends on **BOTH** size and density. Here's the detailed breakdown:

---

## 📊 Algorithm Performance by Graph Type

### 1. **Small Graphs (< 100 vertices)**

#### ✅ **ALL ALGORITHMS WORK**
- Basic Bron-Kerbosch: Yes, works fine
- Tomita (BK with pivoting): Yes, efficient
- Degeneracy BK: Yes, very efficient
- Östergård: Yes, works well
- CPU-Optimized: Yes, fastest

**Typical Runtime:** Milliseconds to seconds

**Example:**
```
Graph: 50 vertices, 200 edges (density: 0.16)
- Basic BK: ~100ms
- Tomita: ~10ms
- Degeneracy BK: ~5ms
- CPU-Optimized: ~2ms
```

---

### 2. **Medium Sparse Graphs (100-1000 vertices, density < 0.01)**

#### ✅ **EXACT ALGORITHMS WORK WELL**

**Recommended:**
- Degeneracy BK: **BEST CHOICE** for sparse graphs
- Tomita: Good alternative
- CPU-Optimized: Fast if < 1024 vertices

**Avoid:**
- Basic BK: Too slow, no pivoting

**Typical Runtime:** Seconds to minutes

**Example:**
```
Graph: 500 vertices, 2000 edges (density: 0.016)
Degeneracy: 8

- Basic BK: ~30 seconds (DON'T USE)
- Tomita: ~5 seconds
- Degeneracy BK: ~1 second (BEST)
- CPU-Optimized: ~0.5 seconds
```

---

### 3. **Medium Dense Graphs (100-500 vertices, density > 0.1)**

#### ⚠️ **EXACT ALGORITHMS: SLOW BUT POSSIBLE**

**Reality Check:**
- Basic BK: Will timeout (minutes to hours)
- Tomita: Slow but usable (10s to minutes)
- Degeneracy BK: Not much better on dense graphs
- Östergård: Good pruning, but still slow
- CPU-Optimized: Best exact option

**Recommended Approach:**
1. Try heuristics first (seconds)
2. If you need exact, use CPU-Optimized (< 1024 vertices)
3. Be prepared to wait

**Example:**
```
Graph: 200 vertices, 8000 edges (density: 0.40)

Heuristics (FAST):
- Greedy: ~50ms
- Simulated Annealing: ~10 seconds
- Randomized Heuristic: ~5 seconds

Exact (SLOW):
- Basic BK: ~10 minutes (DON'T USE)
- Tomita: ~2 minutes
- CPU-Optimized: ~30 seconds (BEST EXACT)
```

---

### 4. **Large Sparse Graphs (> 1000 vertices, density < 0.01)**

#### ⚠️ **EXACT ALGORITHMS: DEGENERACY BK ONLY**

**Reality:**
- Most exact algorithms will be too slow
- Degeneracy BK can work if degeneracy is small (< 20)
- CPU-Optimized limited to 1024 vertices

**Recommended:**
1. **Check degeneracy first:**
   ```cpp
   int d = g.get_degeneracy();
   if (d < 15) {
       // Degeneracy BK might work (try it)
   } else {
       // Use heuristics only
   }
   ```

2. **Use heuristics** for initial solution
3. **Try Degeneracy BK** with timeout

**Example:**
```
Graph: 5000 vertices, 25000 edges (density: 0.002)
Degeneracy: 12

Heuristics (WORK FINE):
- Greedy: ~500ms
- Simulated Annealing: ~30 seconds
- Randomized Heuristic: ~20 seconds

Exact:
- Degeneracy BK: ~5 minutes (if degeneracy < 15)
- Others: Impractical (hours+)
```

---

### 5. **Large Dense Graphs (> 1000 vertices, density > 0.01)**

#### ❌ **EXACT ALGORITHMS: DON'T USE**

**Reality:**
- Exact algorithms will take hours to days
- Exponential explosion makes it impractical
- Only heuristics are feasible

**What to Do:**
```
DON'T: Try to run Bron-Kerbosch
DO: Use fast heuristics
```

**Example:**
```
Graph: 2000 vertices, 500000 edges (density: 0.25)

Heuristics (ONLY OPTION):
- Greedy: ~2 seconds
- Simulated Annealing: ~2 minutes
- Randomized Heuristic: ~1 minute

Exact algorithms: DAYS to NEVER
```

---

## 📈 Performance Table Summary

| Graph Type | Vertices | Density | Degeneracy | Recommended Algorithms | Expected Time |
|------------|----------|---------|------------|----------------------|---------------|
| **Tiny** | < 50 | Any | Any | All algorithms | Milliseconds |
| **Small** | 50-100 | Any | Any | All algorithms | < 1 second |
| **Medium Sparse** | 100-1000 | < 0.01 | < 15 | Degeneracy BK, Tomita, CPU-Opt | Seconds |
| **Medium Dense** | 100-500 | > 0.1 | > 50 | CPU-Opt (if <1024), Heuristics | Seconds to minutes |
| **Large Sparse** | > 1000 | < 0.01 | < 15 | Degeneracy BK (cautious), Heuristics | Minutes |
| **Large Dense** | > 1000 | > 0.01 | > 20 | **Heuristics ONLY** | Seconds to minutes |
| **Very Large** | > 10000 | Any | Any | **Heuristics ONLY** | Seconds to minutes |

---

## 🎯 Decision Tree: Which Algorithm to Use?

```
START
  │
  ├─ Vertices < 100?
  │   └─ YES → Try ALL algorithms (fastest: CPU-Optimized)
  │   
  ├─ Vertices < 1000 AND Density < 0.01?
  │   └─ YES → Use Degeneracy BK (check degeneracy first)
  │   
  ├─ Vertices < 1024 AND need exact?
  │   └─ YES → Use CPU-Optimized (be patient)
  │   
  └─ Otherwise?
      └─ Use Heuristics ONLY:
          1. Greedy (fastest, decent quality)
          2. Randomized Heuristic (better quality)
          3. Simulated Annealing (best quality, slower)
```

---

## 🔍 How to Check Your Graph Before Running

### Script to Analyze Your Graph

Add this to your benchmark or create a separate analysis tool:

```cpp
void analyze_graph_before_benchmark(const Graph& g) {
    int n = g.num_vertices();
    int m = g.num_edges();
    double density = g.get_density();
    int degeneracy = g.get_degeneracy();
    
    std::cout << "=== GRAPH ANALYSIS ===" << std::endl;
    std::cout << "Vertices: " << n << std::endl;
    std::cout << "Edges: " << m << std::endl;
    std::cout << "Density: " << density << std::endl;
    std::cout << "Degeneracy: " << degeneracy << std::endl;
    
    std::cout << "\n=== RECOMMENDATIONS ===" << std::endl;
    
    if (n < 100) {
        std::cout << "✅ Graph is small - ALL algorithms will work" << std::endl;
    } else if (n < 1000 && density < 0.01 && degeneracy < 15) {
        std::cout << "✅ Use Degeneracy BK or Tomita" << std::endl;
    } else if (n < 1024 && density < 0.2) {
        std::cout << "⚠️  Use CPU-Optimized (may be slow)" << std::endl;
    } else if (density > 0.1) {
        std::cout << "❌ Graph is too dense - use HEURISTICS ONLY" << std::endl;
    } else if (n > 1000) {
        std::cout << "❌ Graph is too large - use HEURISTICS ONLY" << std::endl;
    }
    
    if (degeneracy > 20) {
        std::cout << "⚠️  High degeneracy - exact algorithms will be very slow" << std::endl;
    }
}
```

---

## 📊 Real-World Dataset Examples

### SNAP Datasets Analysis

| Dataset | Vertices | Edges | Density | Degeneracy | Can Run Exact? |
|---------|----------|-------|---------|------------|----------------|
| **ego-Facebook** | 4,039 | 88,234 | 0.0108 | 115 | ❌ No (high degeneracy) |
| **ca-GrQc** | 5,242 | 14,496 | 0.0011 | 81 | ❌ No (high degeneracy) |
| **ca-HepTh** | 9,877 | 25,998 | 0.0005 | 31 | ⚠️ Maybe (if timeout) |
| **email-Enron** | 36,692 | 183,831 | 0.0003 | 43 | ❌ No (too large) |
| **soc-Slashdot** | 77,360 | 469,180 | 0.0002 | 51 | ❌ No (too large) |

**Reality:** Most real-world social networks have high degeneracy → **Use heuristics**

---

## 💡 Practical Recommendations

### For Your Course Project

1. **Small Test Graphs (< 100 vertices):**
   - Use ALL algorithms to show comparison
   - Demonstrates algorithm differences clearly
   - All will finish in reasonable time

2. **Medium Graphs (100-1000 vertices):**
   - Focus on Degeneracy BK, Tomita, CPU-Optimized
   - Show when exact algorithms become impractical
   - Compare with heuristics

3. **Large Graphs (> 1000 vertices):**
   - Use heuristics only
   - Show how quality vs speed tradeoff matters
   - Demonstrate scalability

### For Benchmarking

```cpp
// In benchmark.cpp - already implemented!
if (graph.num_vertices() <= 100) {
    // Run all algorithms
    benchmark_algorithm(graph, name, "BronKerbosch", timeout);
    benchmark_algorithm(graph, name, "Tomita", timeout);
    benchmark_algorithm(graph, name, "DegeneracyBK", timeout);
    benchmark_algorithm(graph, name, "Ostergard", timeout);
    benchmark_algorithm(graph, name, "CPUOptimized", timeout);
} else {
    std::cout << "Skipping exact algorithms (graph too large)" << std::endl;
}

// Always run heuristics
benchmark_algorithm(graph, name, "Greedy", timeout);
benchmark_algorithm(graph, name, "SimulatedAnnealing", timeout);
benchmark_algorithm(graph, name, "RandomizedHeuristic", timeout);
```

---

## 🚨 Warning Signs: Don't Run Exact Algorithms If...

1. ❌ Density > 0.1 AND Vertices > 500
2. ❌ Degeneracy > 50
3. ❌ Vertices > 1000 (unless degeneracy < 10)
4. ❌ Graph has > 1 million edges
5. ❌ Previous run didn't finish in 5 minutes

**What happens if you ignore warnings:**
- Your program will run for hours/days
- Might run out of memory
- You'll waste time waiting

**What to do instead:**
- Use heuristics
- Generate smaller test graphs
- Use sparse graphs for exact algorithms

---

## 📝 Summary

### Input Format
- **Simple edge list**: `vertex1 vertex2` per line
- Comments start with `#`
- Space or tab separated
- Automatically handles undirected conversion

### Running Bron-Kerbosch and Variants

| Graph Size | Density | Answer | Best Algorithm |
|------------|---------|--------|----------------|
| < 100 vertices | Any | ✅ YES | Any exact algorithm |
| < 1000 vertices | < 0.01 | ✅ YES | Degeneracy BK |
| < 1024 vertices | < 0.1 | ⚠️ MAYBE | CPU-Optimized |
| > 1000 vertices | Any | ❌ NO | Heuristics only |
| Any size | > 0.1 | ❌ NO | Heuristics only |

**Key Insight:** Degeneracy matters more than size for sparse graphs!

---

## 🔗 See Also

- `README.md` - Full algorithm details
- `QUICKSTART.md` - How to run benchmarks
- `PROJECT_SUMMARY.md` - Algorithm complexity analysis
- `DATASETS.md` - Available datasets and their properties

---

**TL;DR:** 
- Small graphs (< 100 vertices): Run everything
- Medium sparse graphs: Use Degeneracy BK
- Large or dense graphs: Heuristics only
- Always check degeneracy before using exact algorithms!
