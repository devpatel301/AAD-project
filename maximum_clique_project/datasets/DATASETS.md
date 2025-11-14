# Dataset Information and Download Instructions

## Quick Start

```bash
cd datasets
./download_datasets.sh  # Use the script below
```

## SNAP Stanford Network Datasets

### Small Graphs (< 10K vertices) - All Algorithms

#### 1. Facebook Social Circles (ego-Facebook)
- **Vertices**: 4,039
- **Edges**: 88,234
- **Type**: Social network
- **Description**: Anonymized Facebook friend lists
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/facebook_combined.txt.gz
  gunzip facebook_combined.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/ego-Facebook.html

#### 2. General Relativity Collaboration (ca-GrQc)
- **Vertices**: 5,242
- **Edges**: 14,496
- **Type**: Collaboration network
- **Description**: ArXiv GR-QC (General Relativity) collaboration network
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/ca-GrQc.txt.gz
  gunzip ca-GrQc.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/ca-GrQc.html

---

### Medium Graphs (10K-50K vertices) - Heuristics + Fast Exact

#### 3. High Energy Physics Theory (ca-HepTh)
- **Vertices**: 9,877
- **Edges**: 25,998
- **Type**: Collaboration network
- **Description**: ArXiv HEP-TH (High Energy Physics Theory) collaboration
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/ca-HepTh.txt.gz
  gunzip ca-HepTh.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/ca-HepTh.html

#### 4. Enron Email Network
- **Vertices**: 36,692
- **Edges**: 183,831
- **Type**: Communication network
- **Description**: Email communication network from Enron corpus
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/email-Enron.txt.gz
  gunzip email-Enron.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/email-Enron.html

#### 5. High Energy Physics Phenomenology (ca-HepPh)
- **Vertices**: 12,008
- **Edges**: 118,521
- **Type**: Collaboration network
- **Description**: ArXiv HEP-PH collaboration network
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/ca-HepPh.txt.gz
  gunzip ca-HepPh.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/ca-HepPh.html

---

### Large Graphs (50K+ vertices) - Heuristics Only

#### 6. Slashdot Social Network (soc-Slashdot0811)
- **Vertices**: 77,360
- **Edges**: 469,180
- **Type**: Social network
- **Description**: Slashdot friend/foe network from November 2008
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/soc-Slashdot0811.txt.gz
  gunzip soc-Slashdot0811.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/soc-Slashdot0811.html

#### 7. Epinions Social Network
- **Vertices**: 75,879
- **Edges**: 405,740
- **Type**: Social network
- **Description**: Who-trusts-whom network from Epinions.com
- **Download**:
  ```bash
  wget http://snap.stanford.edu/data/soc-Epinions1.txt.gz
  gunzip soc-Epinions1.txt.gz
  ```
- **URL**: http://snap.stanford.edu/data/soc-Epinions1.html

---

## Automated Download Script

Create `download_datasets.sh` in the datasets directory:

```bash
#!/bin/bash

echo "Downloading SNAP datasets..."

# Small graphs
echo "Downloading Facebook dataset..."
wget -q --show-progress http://snap.stanford.edu/data/facebook_combined.txt.gz
gunzip -f facebook_combined.txt.gz

echo "Downloading ca-GrQc dataset..."
wget -q --show-progress http://snap.stanford.edu/data/ca-GrQc.txt.gz
gunzip -f ca-GrQc.txt.gz

# Medium graphs
echo "Downloading ca-HepTh dataset..."
wget -q --show-progress http://snap.stanford.edu/data/ca-HepTh.txt.gz
gunzip -f ca-HepTh.txt.gz

echo "Downloading email-Enron dataset..."
wget -q --show-progress http://snap.stanford.edu/data/email-Enron.txt.gz
gunzip -f email-Enron.txt.gz

# Uncomment for large graphs
# echo "Downloading soc-Slashdot0811 dataset..."
# wget -q --show-progress http://snap.stanford.edu/data/soc-Slashdot0811.txt.gz
# gunzip -f soc-Slashdot0811.txt.gz

echo "Download complete!"
```

Make it executable:
```bash
chmod +x download_datasets.sh
./download_datasets.sh
```

---

## Dataset Format

All SNAP datasets use edge list format:

```
# Comment lines start with hash
# FromNodeId    ToNodeId
0       1
0       2
1       2
...
```

**Important Notes**:
1. Lines starting with `#` are comments (automatically skipped)
2. Edges are space or tab separated
3. The program converts to undirected graphs (adds both directions)
4. Self-loops are ignored
5. Vertex IDs can be arbitrary (automatically remapped)

---

## Alternative Dataset Sources

### 1. Network Repository
- **URL**: http://networkrepository.com/
- **Description**: Large collection of networks across many domains
- **Format**: Various (may need conversion)

### 2. KONECT (Koblenz Network Collection)
- **URL**: http://konect.cc/
- **Description**: Large network collection
- **Format**: Edge list (compatible)

### 3. Laboratory for Web Algorithmics
- **URL**: http://law.di.unimi.it/datasets.php
- **Description**: Web graphs and social networks
- **Format**: Various

### 4. DIMACS Clique Benchmarks
- **URL**: http://iridia.ulb.ac.be/~fmascia/maximum_clique/DIMACS-benchmark
- **Description**: Standard benchmarks for clique algorithms
- **Format**: DIMACS format (requires conversion)

---

## Converting Other Formats

### From DIMACS to Edge List

DIMACS format:
```
p edge 5 7
e 1 2
e 1 3
...
```

Conversion script (`dimacs_to_edgelist.py`):
```python
import sys

with open(sys.argv[1], 'r') as f:
    for line in f:
        if line.startswith('e '):
            _, u, v = line.strip().split()
            print(f"{int(u)-1} {int(v)-1}")  # Convert to 0-indexed
```

Usage:
```bash
python dimacs_to_edgelist.py graph.clq > graph.txt
```

### From Adjacency Matrix

```python
import numpy as np

# Load adjacency matrix
adj = np.loadtxt('matrix.txt', dtype=int)
n = adj.shape[0]

# Convert to edge list
with open('graph.txt', 'w') as f:
    for i in range(n):
        for j in range(i+1, n):
            if adj[i][j]:
                f.write(f"{i} {j}\n")
```

---

## Dataset Statistics After Loading

The program automatically computes and displays:
- Number of vertices
- Number of edges
- Graph density: `2*E / (V*(V-1))`
- Degeneracy: minimum k such that every subgraph has a vertex of degree ≤ k

Example output:
```
Loaded graph: 4039 vertices, 88234 edges
Benchmarking dataset: facebook
  Vertices: 4039
  Edges: 88234
  Density: 0.010837
  Degeneracy: 115
```

---

## Troubleshooting

### "Cannot open file" error
- Check file exists in `datasets/` directory
- Verify filename matches exactly (case-sensitive)
- Ensure file is not still gzipped

### "No valid edges found" error
- File might be empty or corrupt
- Check file format matches edge list format
- Try opening in text editor to verify

### Out of memory
- Graph too large for available RAM
- Try heuristic algorithms only
- Consider using smaller dataset

---

## Creating Custom Datasets

### Random Graph Generator

```cpp
// In your code
Graph generate_random_graph(int n, double edge_prob) {
    Graph g(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < n; i++) {
        for (int j = i+1; j < n; j++) {
            if (dis(gen) < edge_prob) {
                g.add_edge(i, j);
            }
        }
    }
    return g;
}
```

### Synthetic Clique-Heavy Graph

```cpp
// Graph with known maximum clique
Graph generate_clique_graph(int clique_size, int total_vertices, double noise) {
    Graph g(total_vertices);
    
    // Create clique
    for (int i = 0; i < clique_size; i++) {
        for (int j = i+1; j < clique_size; j++) {
            g.add_edge(i, j);
        }
    }
    
    // Add random edges
    for (int i = 0; i < total_vertices; i++) {
        for (int j = i+1; j < total_vertices; j++) {
            if (dis(gen) < noise) {
                g.add_edge(i, j);
            }
        }
    }
    
    return g;
}
```

---

## Recommended Datasets for Different Purposes

### Testing Correctness (Small, Known Solutions)
- Create small synthetic graphs with known cliques
- DIMACS benchmarks have known optimal solutions

### Performance Benchmarking
- **Sparse**: ca-GrQc, ca-HepTh
- **Dense**: Facebook combined
- **Large**: Slashdot, Epinions

### Algorithm Comparison
Use variety:
1. Very sparse (degeneracy < 10)
2. Moderate (degeneracy 10-50)
3. Dense (degeneracy > 100)

---

For more information, visit the [SNAP website](http://snap.stanford.edu/data/).
