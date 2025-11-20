#!/usr/bin/env python3
"""
Synthetic Graph Generator for Maximum Clique Benchmarking

Generates graphs using:
1. R-MAT algorithm (Recursive MATrix) for random and skewed degree distributions
2. 3-SAT to graph conversion for cryptographic-inspired graphs

Graph Categories:
- Random graphs (Erdős-Rényi via R-MAT): (0.25, 0.25, 0.25, 0.25)
- Skewed Degree Type 1: (0.45, 0.15, 0.15, 0.25)
- Skewed Degree Type 2: (0.55, 0.15, 0.15, 0.15)
- 3-SAT derived graphs: k-partite graphs from cryptographic hash SAT problems
"""

import numpy as np
import random
import hashlib
from collections import defaultdict
import os


class RMATGraphGenerator:
    """
    R-MAT (Recursive MATrix) Graph Generator
    
    Generates graphs by recursively partitioning adjacency matrix into quadrants
    and probabilistically placing edges based on parameters (a, b, c, d).
    
    Parameters sum to 1.0:
    - a: probability for top-left quadrant
    - b: probability for top-right quadrant  
    - c: probability for bottom-left quadrant
    - d: probability for bottom-right quadrant
    """
    
    def __init__(self, n_vertices, n_edges, a, b, c, d, seed=None):
        """
        Initialize R-MAT generator
        
        Args:
            n_vertices: Number of vertices (will be rounded up to power of 2)
            n_edges: Target number of edges
            a, b, c, d: R-MAT parameters (should sum to 1.0)
            seed: Random seed for reproducibility
        """
        self.n_vertices = n_vertices
        self.n_edges = n_edges
        self.a = a
        self.b = b
        self.c = c
        self.d = d
        
        if seed is not None:
            np.random.seed(seed)
            random.seed(seed)
        
        # Round up to nearest power of 2 for R-MAT
        self.n = 2 ** int(np.ceil(np.log2(n_vertices)))
        
    def generate_edge(self):
        """Generate a single edge using R-MAT algorithm"""
        u, v = 0, 0
        
        # Start with full matrix
        for depth in range(int(np.log2(self.n))):
            # Randomly choose quadrant based on probabilities
            rand = np.random.random()
            
            bit_u, bit_v = 0, 0
            
            if rand < self.a:
                # Top-left quadrant
                bit_u, bit_v = 0, 0
            elif rand < self.a + self.b:
                # Top-right quadrant
                bit_u, bit_v = 0, 1
            elif rand < self.a + self.b + self.c:
                # Bottom-left quadrant
                bit_u, bit_v = 1, 0
            else:
                # Bottom-right quadrant
                bit_u, bit_v = 1, 1
            
            # Update vertex indices
            u = (u << 1) | bit_u
            v = (v << 1) | bit_v
        
        return u % self.n_vertices, v % self.n_vertices
    
    def generate(self):
        """Generate complete graph using R-MAT"""
        edges = set()
        attempts = 0
        max_attempts = self.n_edges * 10  # Prevent infinite loops
        
        while len(edges) < self.n_edges and attempts < max_attempts:
            u, v = self.generate_edge()
            
            # Skip self-loops
            if u != v:
                # Add edge (undirected, so store in canonical order)
                edge = (min(u, v), max(u, v))
                edges.add(edge)
            
            attempts += 1
        
        return edges


class SAT3GraphGenerator:
    """
    3-SAT to Graph Converter
    
    Creates a k-partite graph from a 3-SAT problem derived from cryptographic hash.
    The graph structure encodes the SAT problem constraints.
    """
    
    def __init__(self, n_vertices, density_target=0.1, seed=None):
        """
        Initialize 3-SAT graph generator
        
        Args:
            n_vertices: Target number of vertices (< 500 as specified)
            density_target: Target graph density (0.1 = 10%)
            seed: Random seed for reproducibility
        """
        self.n_vertices = min(n_vertices, 500)  # Cap at 500 vertices
        self.density_target = density_target
        
        if seed is not None:
            np.random.seed(seed)
            random.seed(seed)
    
    def generate_sat_problem(self, n_vars, n_clauses):
        """
        Generate a 3-SAT problem using cryptographic hash
        
        Creates clauses by hashing variable combinations and using
        hash values to determine clause structure
        """
        clauses = []
        
        for i in range(n_clauses):
            # Use hash to generate clause
            hash_input = f"clause_{i}_{n_vars}".encode()
            hash_val = hashlib.sha256(hash_input).digest()
            
            # Extract 3 variables and their polarities from hash
            var1 = int.from_bytes(hash_val[0:2], 'big') % n_vars + 1
            var2 = int.from_bytes(hash_val[2:4], 'big') % n_vars + 1
            var3 = int.from_bytes(hash_val[4:6], 'big') % n_vars + 1
            
            # Ensure distinct variables
            while var2 == var1:
                var2 = (var2 + 1) % n_vars + 1
            while var3 == var1 or var3 == var2:
                var3 = (var3 + 1) % n_vars + 1
            
            # Polarities (positive/negative)
            pol1 = 1 if hash_val[6] % 2 == 0 else -1
            pol2 = 1 if hash_val[7] % 2 == 0 else -1
            pol3 = 1 if hash_val[8] % 2 == 0 else -1
            
            clauses.append([(var1, pol1), (var2, pol2), (var3, pol3)])
        
        return clauses
    
    def sat_to_graph(self, n_vars, clauses):
        """
        Convert 3-SAT problem to k-partite graph
        
        Creates a vertex for each literal (variable and its negation)
        Edges connect literals that cannot both be true
        """
        # Create vertices for each literal (positive and negative)
        vertices = {}
        vertex_id = 0
        
        # Map literals to vertex IDs
        for var in range(1, n_vars + 1):
            vertices[(var, 1)] = vertex_id  # Positive literal
            vertex_id += 1
            vertices[(var, -1)] = vertex_id  # Negative literal
            vertex_id += 1
        
        edges = set()
        
        # Add edges between conflicting literals
        # 1. Variable and its negation are always in conflict
        for var in range(1, n_vars + 1):
            u = vertices[(var, 1)]
            v = vertices[(var, -1)]
            edges.add((min(u, v), max(u, v)))
        
        # 2. Literals from same clause that would make clause false
        for clause in clauses:
            for i in range(len(clause)):
                for j in range(i + 1, len(clause)):
                    lit1, lit2 = clause[i], clause[j]
                    # Add edge with some probability (controls density)
                    if random.random() < 0.3:
                        u = vertices[lit1]
                        v = vertices[lit2]
                        edges.add((min(u, v), max(u, v)))
        
        # Limit to target vertex count
        n_final = min(len(vertices), self.n_vertices)
        
        # Filter edges to only include vertices in range
        edges = {(u, v) for u, v in edges if u < n_final and v < n_final}
        
        # Adjust density by randomly removing edges
        target_edges = int(n_final * (n_final - 1) / 2 * self.density_target)
        edges_list = list(edges)
        
        if len(edges_list) > target_edges:
            edges = set(random.sample(edges_list, target_edges))
        
        return n_final, edges
    
    def generate(self):
        """Generate graph from 3-SAT problem"""
        # Calculate number of variables and clauses
        # Each variable generates 2 literals (positive and negative)
        n_vars = self.n_vertices // 2
        n_clauses = n_vars * 2  # Ratio of clauses to variables
        
        # Generate SAT problem
        clauses = self.generate_sat_problem(n_vars, n_clauses)
        
        # Convert to graph
        n_vertices, edges = self.sat_to_graph(n_vars, clauses)
        
        return n_vertices, edges


def save_graph_snap_format(edges, n_vertices, filename):
    """
    Save graph in SNAP format
    
    Format:
    # Comment lines
    source_vertex target_vertex
    """
    with open(filename, 'w') as f:
        f.write(f"# Synthetic graph for maximum clique benchmarking\n")
        f.write(f"# Vertices: {n_vertices}\n")
        f.write(f"# Edges: {len(edges)}\n")
        f.write(f"# Undirected graph (each edge listed once)\n")
        
        for u, v in sorted(edges):
            f.write(f"{u} {v}\n")
    
    print(f"Generated {filename}: {n_vertices} vertices, {len(edges)} edges")


def generate_all_graphs():
    """Generate all synthetic graphs"""
    
    output_dir = "datasets"
    os.makedirs(output_dir, exist_ok=True)
    
    print("=" * 70)
    print("SYNTHETIC GRAPH GENERATION")
    print("=" * 70)
    
    # ========================================================================
    # 1. Random Graphs (Erdős-Rényi via R-MAT)
    # ========================================================================
    print("\n1. Generating Random Graphs (R-MAT with ER parameters)...")
    print("   Parameters: a=0.25, b=0.25, c=0.25, d=0.25")
    
    # Small random graph
    gen = RMATGraphGenerator(
        n_vertices=200,
        n_edges=2000,
        a=0.25, b=0.25, c=0.25, d=0.25,
        seed=42
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices, 
                          f"{output_dir}/rmat_er_small.txt")
    
    # Large random graph
    gen = RMATGraphGenerator(
        n_vertices=500,
        n_edges=8000,
        a=0.25, b=0.25, c=0.25, d=0.25,
        seed=43
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices,
                          f"{output_dir}/rmat_er_large.txt")
    
    # ========================================================================
    # 2. Skewed Degree Type 1 Graphs
    # ========================================================================
    print("\n2. Generating Skewed Degree Type 1 Graphs...")
    print("   Parameters: a=0.45, b=0.15, c=0.15, d=0.25")
    
    # Small skewed type 1
    gen = RMATGraphGenerator(
        n_vertices=200,
        n_edges=2500,
        a=0.45, b=0.15, c=0.15, d=0.25,
        seed=44
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices,
                          f"{output_dir}/rmat_sd1_small.txt")
    
    # Large skewed type 1
    gen = RMATGraphGenerator(
        n_vertices=500,
        n_edges=10000,
        a=0.45, b=0.15, c=0.15, d=0.25,
        seed=45
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices,
                          f"{output_dir}/rmat_sd1_large.txt")
    
    # ========================================================================
    # 3. Skewed Degree Type 2 Graphs
    # ========================================================================
    print("\n3. Generating Skewed Degree Type 2 Graphs...")
    print("   Parameters: a=0.55, b=0.15, c=0.15, d=0.15")
    
    # Small skewed type 2
    gen = RMATGraphGenerator(
        n_vertices=200,
        n_edges=2500,
        a=0.55, b=0.15, c=0.15, d=0.15,
        seed=46
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices,
                          f"{output_dir}/rmat_sd2_small.txt")
    
    # Large skewed type 2
    gen = RMATGraphGenerator(
        n_vertices=500,
        n_edges=10000,
        a=0.55, b=0.15, c=0.15, d=0.15,
        seed=47
    )
    edges = gen.generate()
    save_graph_snap_format(edges, gen.n_vertices,
                          f"{output_dir}/rmat_sd2_large.txt")
    
    # ========================================================================
    # 4. 3-SAT Derived Graphs
    # ========================================================================
    print("\n4. Generating 3-SAT Derived Graphs...")
    print("   Cryptographic hash-based SAT problems converted to k-partite graphs")
    
    # Small 3-SAT graph (low density)
    gen = SAT3GraphGenerator(
        n_vertices=150,
        density_target=0.08,
        seed=48
    )
    n_vertices, edges = gen.generate()
    save_graph_snap_format(edges, n_vertices,
                          f"{output_dir}/sat3_small.txt")
    
    # Large 3-SAT graph (moderate density)
    gen = SAT3GraphGenerator(
        n_vertices=300,
        density_target=0.12,
        seed=49
    )
    n_vertices, edges = gen.generate()
    save_graph_snap_format(edges, n_vertices,
                          f"{output_dir}/sat3_large.txt")
    
    print("\n" + "=" * 70)
    print("GENERATION COMPLETE!")
    print("=" * 70)
    print("\nGenerated 8 synthetic graphs:")
    print("  - 2 Random (Erdős-Rényi)")
    print("  - 2 Skewed Degree Type 1")
    print("  - 2 Skewed Degree Type 2")
    print("  - 2 3-SAT Derived")


if __name__ == "__main__":
    generate_all_graphs()
