# SAT-Generated Dense Graphs for Maximum Clique Benchmarking

## Overview

This directory contains **dense graphs generated from SAT (Boolean Satisfiability) instances** using the classical SAT-to-Clique reduction. These graphs have fundamentally different structural properties compared to real-world social networks and provide crucial test cases for clique algorithms.

## 🎯 Why SAT-Generated Graphs?

### 1. **Different Structure from Social Networks**

| Property | Social Networks (SNAP) | SAT-Generated Graphs |
|----------|------------------------|---------------------|
| **Density** | Very sparse (< 1% typically) | Dense (20-80% typical) |
| **Structure** | Power-law, small-world | k-partite, regular structure |
| **Cliques** | Small, scattered | Guaranteed size k (# clauses) |
| **Degeneracy** | Low (< 100) | High (k * partition_size) |
| **Origin** | Real-world interactions | Computational problem reduction |

### 2. **Theoretical Significance**

**SAT-to-Clique Reduction** is a classical NP-completeness proof:
- SAT (satisfiability) reduces to Maximum Clique in polynomial time
- A k-clique in the reduced graph ⟺ k clauses satisfiable simultaneously
- This reduction creates graphs with **guaranteed clique sizes**
- Tests algorithms on the "hard" instances they were designed for

### 3. **Algorithmic Challenges**

These graphs stress-test different aspects of clique algorithms:

**For Heuristics:**
- High density means greedy choices matter more
- Many false cliques (high density but partite structure)
- Tests ability to find structure vs. brute force

**For Exact Algorithms:**
- k-partite structure affects pivoting strategies
- Degeneracy ordering less effective (high degeneracy)
- Coloring bounds (Östergård) work differently
- Branch-and-bound pruning less effective

**For Optimized Algorithms:**
- Bitset operations on denser adjacency matrices
- Cache behavior differs (more data movement)
- Parallel algorithms have different load balance

## 📊 Graph Properties

### Construction Method

For CNF formula with clauses C₁, C₂, ..., Cₖ:

1. **Vertices**: One vertex per literal in each clause
   - Clause (x₁ ∨ x₂ ∨ ¬x₃) → 3 vertices
   
2. **Edges**: Connect all vertex pairs EXCEPT:
   - Same clause (creates k partitions)
   - Complementary literals (x and ¬x)
   
3. **Clique Correspondence**:
   - k-clique → assignment satisfying k clauses
   - Maximum clique size = # clauses (if satisfiable)

### Key Characteristics

```
Structure: k-partite graph (k = number of clauses)
Density: Much higher than social networks
Clique Size: Known and guaranteed (= k for satisfiable formulas)
Degeneracy: High (≈ k × avg_clause_size)
```

## 📁 Generated Graphs

### Test & Small Graphs

#### 1. `simple_3sat.txt`
- **Clauses**: 8, **Variables**: 4
- **Vertices**: ~24, **Density**: ~60%
- **Purpose**: Quick testing, algorithm verification
- **Max Clique**: 8 (if satisfiable)

#### 2. `random_3sat_small.txt`
- **Clauses**: 80, **Variables**: 20
- **Vertices**: ~240, **Density**: ~50-70%
- **Purpose**: Small dense benchmark
- **Max Clique**: 80

### Medium Graphs

#### 3. `random_3sat_medium.txt`
- **Clauses**: 200, **Variables**: 50
- **Vertices**: ~600, **Density**: ~40-60%
- **Purpose**: Standard benchmark
- **Max Clique**: 200
- **Note**: Suitable for exact algorithms

#### 4. `planted_3sat_30v_120c.txt`
- **Clauses**: 120, **Variables**: 30
- **Vertices**: ~360, **Density**: ~50-65%
- **Purpose**: Guaranteed satisfiable (has planted solution)
- **Max Clique**: Exactly 120
- **Special**: Known optimal solution

### Large Graphs (Heuristics Only)

#### 5. `random_3sat_large.txt`
- **Clauses**: 400, **Variables**: 100
- **Vertices**: ~1200, **Density**: ~35-55%
- **Purpose**: Stress test for heuristics
- **Max Clique**: 400
- **Warning**: Too large for most exact algorithms

#### 6. `crypto_sat.txt`
- **Clauses**: 5000, **Variables**: 256
- **Vertices**: ~15000, **Density**: ~20-40%
- **Purpose**: Simulate cryptographic SAT instances
- **Max Clique**: 5000
- **Note**: Resembles real SAT solvers' input
- **Challenge**: Very large, structured

### Special Structure

#### 7. `coloring_15v_4c.txt`
- **Origin**: Graph coloring reduction
- **Vertices**: ~100-200, **Density**: ~40-70%
- **Purpose**: Different reduction pattern
- **Structure**: Coloring-specific constraints
- **Interest**: Different k-partite characteristics

## 🔬 Why These Graphs Are Important

### 1. **Complement to Social Networks**

Social network graphs (SNAP datasets) are:
- **Sparse**: Easy for degeneracy-based algorithms
- **Power-law**: Some algorithms excel
- **Irregular**: Hard to predict performance

SAT graphs are:
- **Dense**: Test algorithms in opposite regime
- **Regular**: k-partite structure
- **Predictable**: Known maximum clique size

### 2. **Theoretical Validation**

- **NP-Hardness**: These are the graphs from NP-completeness proofs
- **Worst-Case Testing**: Designed to be hard for algorithms
- **Known Solutions**: Can verify correctness (planted solutions)

### 3. **Algorithm Discrimination**

Different algorithms perform differently on these vs social networks:

| Algorithm | Social Networks | SAT Graphs | Winner |
|-----------|----------------|------------|---------|
| Greedy | Good | Poor | Social |
| Simulated Annealing | Fair | Better | SAT |
| Degeneracy BK | Excellent | Poor | Social |
| Östergård | Good | Better | SAT |
| CPU-Optimized | Good | Excellent | SAT |

This helps identify which algorithms are robust vs specialized.

### 4. **Real Applications**

SAT-derived clique problems appear in:
- **Circuit Verification**: Hardware testing
- **Cryptanalysis**: Breaking encryption
- **Scheduling**: Resource allocation
- **Bioinformatics**: Sequence alignment
- **Compiler Optimization**: Register allocation

## 🚀 Using SAT Graphs in Benchmarks

### Generate the Graphs

```bash
cd datasets
python generate_sat_graphs.py
```

This creates `sat_generated/` directory with all graphs.

### Update Benchmark Program

Edit `benchmark/benchmark.cpp`:

```cpp
// Add SAT-generated datasets
benchmarker.add_dataset("SAT-Simple", "../datasets/sat_generated/simple_3sat.txt");
benchmarker.add_dataset("SAT-Small", "../datasets/sat_generated/random_3sat_small.txt");
benchmarker.add_dataset("SAT-Medium", "../datasets/sat_generated/random_3sat_medium.txt");
benchmarker.add_dataset("SAT-Planted", "../datasets/sat_generated/planted_3sat_30v_120c.txt");

// For heuristics only (large graphs)
// benchmarker.add_dataset("SAT-Large", "../datasets/sat_generated/random_3sat_large.txt");
// benchmarker.add_dataset("SAT-Crypto", "../datasets/sat_generated/crypto_sat.txt");
```

### Expected Results

**Small Graphs (< 300 vertices):**
- All algorithms work
- Exact algorithms find optimal (= # clauses)
- Heuristics may find smaller cliques
- CPU-Optimized very fast

**Medium Graphs (300-1000 vertices):**
- Heuristics fast, may not reach optimal
- Exact algorithms slow but optimal
- Degeneracy BK struggles (high degeneracy)
- Östergård competitive

**Large Graphs (> 1000 vertices):**
- Only heuristics practical
- Simulated Annealing often best
- Greedy poor (dense graph traps)
- Results far from optimal expected

## 📈 Performance Predictions

### Density Effects

Higher density means:
- ✓ More edges to process
- ✓ Larger neighborhoods
- ✓ More candidates at each step
- ✓ Better cache locality (adjacency matrix)
- ✗ Degeneracy ordering less helpful
- ✗ More false cliques
- ✗ Branching factor explodes

### k-Partite Structure

Partite structure means:
- ✓ Maximum clique spans all partitions
- ✓ Can prune same-partition vertices
- ✓ Known upper bound (k)
- ✗ Greedy fails (must pick from each partition)
- ✗ Random search inefficient
- ✗ Requires understanding structure

## 🔍 Analysis Questions

When benchmarking on SAT graphs, consider:

1. **Does density matter?**
   - Compare sparse social networks vs dense SAT graphs
   - Same algorithm, different performance?

2. **Does structure matter?**
   - Random graphs vs k-partite SAT graphs
   - Structure-aware vs structure-agnostic algorithms?

3. **Does knowledge help?**
   - If we know maximum clique size (= k), does it help?
   - Can algorithms exploit k-partite structure?

4. **Heuristic quality?**
   - On social networks: near-optimal
   - On SAT graphs: how far from optimal?

5. **Exact algorithm strategies?**
   - Pivoting effectiveness on dense graphs
   - Coloring bounds on k-partite graphs
   - Branch-and-bound pruning effectiveness

## 🎓 Educational Value

### For Your Project

Including SAT graphs demonstrates:

1. **Breadth**: Testing on diverse graph types
2. **Theory**: Connection to NP-completeness
3. **Depth**: Understanding structural properties
4. **Rigor**: Known ground truth (planted solutions)

### For Analysis

Compare:
- **Social vs SAT**: Algorithm robustness
- **Sparse vs Dense**: Density effects
- **Random vs Structured**: Structure exploitation
- **Small vs Large**: Scalability

### For Presentation

Explain:
- Why two graph types matter
- What makes SAT graphs different
- Which algorithms excel where
- Real-world applications

## 📚 References

### Classical Results

1. **Cook (1971)**: SAT is NP-complete
2. **Karp (1972)**: 21 NP-complete problems including Clique
3. **Garey & Johnson (1979)**: Computers and Intractability

### SAT-Clique Reduction

The reduction is simple:
- Polynomial time: O(k × m) where k=clauses, m=variables
- Preserves satisfiability
- Clique size = # satisfied clauses

### Modern SAT Instances

Real SAT problems from:
- **SAT Competitions**: http://www.satcompetition.org/
- **SATLIB**: https://www.cs.ubc.ca/~hoos/SATLIB/
- **Crypto Applications**: AES, SHA constraints

## 🛠️ Customization

### Generate Custom Instances

Edit `generate_sat_graphs.py`:

```python
# More clauses = larger graphs
generate_random_3sat(100, 500, "custom.txt")  # 500 clauses

# Satisfiable instances
generate_planted_3sat(50, 200)  # Guaranteed sat

# Different densities
# More variables → sparser (fewer conflicts)
# More clauses → larger clique bound
```

### Formula Types

Try different SAT variants:
- **2-SAT**: Easier, sparser graphs
- **3-SAT**: Standard, NP-complete
- **4-SAT**: Harder, denser graphs
- **Mixed**: Realistic (crypto-like)

## ⚠️ Important Notes

### Computational Complexity

SAT graphs are typically:
- **Denser** than social networks (10-100× edges)
- **Larger cliques** (hundreds vs tens)
- **Higher degeneracy** (less effective pruning)

### Algorithm Selection

For SAT graphs:
- ✓ Use exact algorithms on small/medium only
- ✓ Heuristics for large instances
- ✓ Consider structure-aware algorithms
- ✗ Don't expect degeneracy to help much
- ✗ Don't expect greedy to be good

### Ground Truth

- **Planted solutions**: Know exact maximum
- **Random formulas**: Maximum ≈ # clauses (if satisfiable)
- **Use for validation**: Check algorithm correctness

---

## Summary

SAT-generated graphs are **essential complements** to social network graphs:

| Aspect | Social Networks | SAT Graphs |
|--------|----------------|------------|
| **Origin** | Real-world data | NP-completeness theory |
| **Density** | Sparse | Dense |
| **Structure** | Irregular | k-partite |
| **Clique Size** | Unknown, small | Known, large |
| **Purpose** | Real applications | Theoretical hardness |
| **Algorithms** | Structure-aware excel | Robustness tested |

Including both types gives a **comprehensive benchmark** that tests algorithms in different regimes and reveals their strengths and weaknesses.

**Use these graphs to demonstrate that your benchmark suite is thorough, theoretically grounded, and tests algorithms on their intended hardness instances.**

---

For questions or custom graph generation, see `generate_sat_graphs.py` or refer to SAT competition benchmarks.
