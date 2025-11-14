#!/usr/bin/env python3
"""
SAT-to-Clique Graph Generator

This script generates dense graph instances by reducing SAT (Boolean satisfiability)
problems to the Maximum Clique Problem. These graphs have special structural
properties that make them valuable benchmarks for clique algorithms.

The reduction works as follows:
1. For each clause in CNF formula, create vertices (one per literal)
2. Connect vertices if they're from different clauses AND not complementary
3. A k-clique in this graph corresponds to a satisfying assignment (k clauses satisfied)

Author: Generated for AAD Algorithms Project
Date: November 2025
"""

import random
import itertools
from typing import List, Tuple, Set

class SATtoCliqueConverter:
    """Convert SAT instances to k-partite clique graphs"""
    
    def __init__(self, cnf_formula: List[List[int]]):
        """
        cnf_formula: List of clauses, each clause is list of literals
        Positive literal = variable index, negative = -variable_index
        Example: [[1, 2, -3], [-1, 3, 4]] means (x1 ∨ x2 ∨ ¬x3) ∧ (¬x1 ∨ x3 ∨ x4)
        """
        self.cnf = cnf_formula
        self.num_clauses = len(cnf_formula)
        self.num_variables = max(abs(lit) for clause in cnf_formula for lit in clause)
        
    def generate_graph(self) -> Tuple[int, List[Tuple[int, int]], List[Set[int]]]:
        """
        Returns: (num_vertices, edge_list, partitions)
        - num_vertices: total vertices in graph
        - edge_list: list of (u, v) edges
        - partitions: list of sets, each set is one partition
        """
        # Assign vertex ID: clause_idx * max_literals_per_clause + position_in_clause
        vertex_to_literal = {}
        partitions = []
        vertex_id = 0
        
        # Create vertices and partitions
        for clause_idx, clause in enumerate(self.cnf):
            partition = set()
            for literal in clause:
                vertex_to_literal[vertex_id] = (clause_idx, literal)
                partition.add(vertex_id)
                vertex_id += 1
            partitions.append(partition)
        
        num_vertices = vertex_id
        edges = []
        
        # Generate edges: connect all pairs EXCEPT same clause or complementary literals
        for u in range(num_vertices):
            for v in range(u + 1, num_vertices):
                clause_u, lit_u = vertex_to_literal[u]
                clause_v, lit_v = vertex_to_literal[v]
                
                # Don't connect if same clause (vertices in same partition)
                if clause_u == clause_v:
                    continue
                
                # Don't connect if complementary literals (would create conflict)
                if lit_u == -lit_v:
                    continue
                
                edges.append((u, v))
        
        return num_vertices, edges, partitions
    
    def write_snap_format(self, filename: str):
        """Write graph in SNAP edge list format (for our benchmark)"""
        num_vertices, edges, partitions = self.generate_graph()
        
        with open(filename, 'w') as f:
            f.write(f"# SAT-to-Clique reduction graph\n")
            f.write(f"# Original SAT: {self.num_clauses} clauses, {self.num_variables} variables\n")
            f.write(f"# Graph: {num_vertices} vertices, {len(edges)} edges\n")
            f.write(f"# Structure: {len(partitions)}-partite graph\n")
            f.write(f"# Maximum clique size: {len(partitions)} (one vertex per partition)\n")
            f.write(f"# FromNodeId\tToNodeId\n")
            
            for u, v in edges:
                f.write(f"{u}\t{v}\n")
        
        density = (2 * len(edges)) / (num_vertices * (num_vertices - 1)) if num_vertices > 1 else 0
        print(f"✓ Generated: {filename}")
        print(f"  Vertices: {num_vertices}, Edges: {len(edges)}")
        print(f"  Density: {density:.4f} ({density*100:.2f}%)")
        print(f"  k-partite: k={len(partitions)}")
        print(f"  Max clique: {len(partitions)}")
        
    def analyze_density(self):
        """Analyze graph density and clique properties"""
        num_vertices, edges, partitions = self.generate_graph()
        max_possible_edges = num_vertices * (num_vertices - 1) // 2
        actual_edges = len(edges)
        
        print(f"\n=== GRAPH PROPERTIES ===")
        print(f"Vertices: {num_vertices}")
        print(f"Max possible edges: {max_possible_edges}")
        print(f"Actual edges: {actual_edges}")
        print(f"Density: {actual_edges/max_possible_edges:.4f}")
        print(f"k-partite with k={len(partitions)}")
        print(f"Max clique size: {len(partitions)} (guaranteed by construction)")
        
        # Calculate average partition size
        avg_partition_size = sum(len(p) for p in partitions) / len(partitions)
        print(f"Avg partition size: {avg_partition_size:.2f}")
        
        # Test random subsets
        print(f"\n=== CLIQUE SAMPLING TEST ===")
        for size in [min(5, len(partitions)), min(10, len(partitions)), 
                     min(20, len(partitions)), min(30, len(partitions))]:
            if size <= len(partitions) and size > 0:
                trials = 100
                clique_count = 0
                edge_set = set(edges)
                edge_set.update((v, u) for u, v in edges)  # Make symmetric
                
                for _ in range(trials):
                    # Sample random vertices from different partitions
                    sample = set()
                    sampled_partitions = random.sample(partitions, size)
                    for partition in sampled_partitions:
                        sample.add(random.choice(list(partition)))
                    
                    # Check if sample is a clique
                    is_clique = True
                    sample_list = list(sample)
                    for i in range(len(sample_list)):
                        for j in range(i+1, len(sample_list)):
                            u, v = sample_list[i], sample_list[j]
                            if (u, v) not in edge_set:
                                is_clique = False
                                break
                        if not is_clique:
                            break
                    
                    if is_clique:
                        clique_count += 1
                
                print(f"Size {size:2d}: {clique_count:3d}/{trials} random samples are cliques "
                      f"({clique_count/trials*100:5.1f}%)")


# Example 1: Generate from simple 3-SAT formula
def generate_simple_3sat():
    """Generate a simple 3-SAT instance for testing"""
    cnf = [
        [1, 2, 3],      # (x1 ∨ x2 ∨ x3)
        [-1, -2, 4],    # (¬x1 ∨ ¬x2 ∨ x4)
        [2, -3, -4],    # (x2 ∨ ¬x3 ∨ ¬x4)
        [1, 3, 4],      # (x1 ∨ x3 ∨ x4)
        [-1, 2, -4],    # (¬x1 ∨ x2 ∨ ¬x4)
        [3, -2, 1],     # (x3 ∨ ¬x2 ∨ x1)
        [-3, 4, -1],    # (¬x3 ∨ x4 ∨ ¬x1)
        [2, 3, -4]      # (x2 ∨ x3 ∨ ¬x4)
    ]
    
    converter = SATtoCliqueConverter(cnf)
    converter.analyze_density()
    converter.write_snap_format("sat_generated/simple_3sat.txt")


# Example 2: Generate random 3-SAT with many clauses
def generate_random_3sat(num_vars: int, num_clauses: int, filename: str):
    """Generate random 3-SAT instance"""
    cnf = []
    for _ in range(num_clauses):
        clause = []
        vars_in_clause = random.sample(range(1, num_vars + 1), min(3, num_vars))
        for var in vars_in_clause:
            clause.append(var if random.random() > 0.5 else -var)
        cnf.append(clause)
    
    converter = SATtoCliqueConverter(cnf)
    converter.analyze_density()
    converter.write_snap_format(f"sat_generated/{filename}")


# Example 3: Generate from cryptographic-style SAT (simulated)
def generate_crypto_like_sat():
    """
    Simulate SAT from cryptographic problem (e.g., SHA preimage)
    Real crypto SAT would have 20,000+ clauses
    
    Characteristics:
    - Many variables (256 for SHA-256)
    - Many clauses (thousands)
    - Mix of clause sizes (2-SAT, 3-SAT, 4-SAT)
    - High constraint density
    """
    num_vars = 256  # SHA-256 has 256-bit input
    num_clauses = 5000  # Simplified; real SHA-256 has ~20,000 clauses
    
    print("Generating crypto-like SAT instance...")
    print(f"Variables: {num_vars}, Clauses: {num_clauses}")
    
    cnf = []
    for _ in range(num_clauses):
        # Mix of 2-SAT, 3-SAT, and 4-SAT clauses (realistic for crypto)
        clause_size = random.choice([2, 3, 3, 3, 4])  # 3-SAT most common
        clause = []
        vars_in_clause = random.sample(range(1, num_vars + 1), min(clause_size, num_vars))
        for var in vars_in_clause:
            clause.append(var if random.random() > 0.5 else -var)
        cnf.append(clause)
    
    converter = SATtoCliqueConverter(cnf)
    converter.analyze_density()
    converter.write_snap_format("sat_generated/crypto_sat.txt")


# Example 4: Generate satisfiable 3-SAT (planted solution)
def generate_planted_3sat(num_vars: int, num_clauses: int):
    """
    Generate 3-SAT with a known satisfying assignment (planted solution)
    This ensures the graph has a maximum clique of size num_clauses
    """
    # Create random assignment
    assignment = {i: random.choice([True, False]) for i in range(1, num_vars + 1)}
    
    cnf = []
    for _ in range(num_clauses):
        clause = []
        vars_in_clause = random.sample(range(1, num_vars + 1), min(3, num_vars))
        
        # Ensure at least one literal in clause is satisfied
        satisfied = False
        for var in vars_in_clause:
            if random.random() > 0.3 and not satisfied:  # With high probability, satisfy
                if assignment[var]:
                    clause.append(var)
                    satisfied = True
                else:
                    clause.append(-var)
                    satisfied = True
            else:
                clause.append(var if random.random() > 0.5 else -var)
        
        # Ensure clause has at least one satisfied literal
        if not any((lit > 0 and assignment[abs(lit)]) or 
                   (lit < 0 and not assignment[abs(lit)]) for lit in clause):
            # Force satisfaction
            var = vars_in_clause[0]
            clause[0] = var if assignment[var] else -var
        
        cnf.append(clause)
    
    converter = SATtoCliqueConverter(cnf)
    converter.analyze_density()
    converter.write_snap_format(f"sat_generated/planted_3sat_{num_vars}v_{num_clauses}c.txt")


# Example 5: Generate graph coloring reduction
def generate_graph_coloring_sat(num_vertices: int, num_colors: int, edge_density: float):
    """
    Generate SAT instance from graph coloring problem
    Creates dense k-partite graphs with interesting structure
    """
    # Generate random graph to color
    edges = []
    for i in range(num_vertices):
        for j in range(i + 1, num_vertices):
            if random.random() < edge_density:
                edges.append((i, j))
    
    # Convert to SAT (standard graph coloring encoding)
    cnf = []
    
    # Each vertex must have at least one color
    for v in range(num_vertices):
        clause = [v * num_colors + c + 1 for c in range(num_colors)]
        cnf.append(clause)
    
    # Adjacent vertices cannot have the same color
    for u, v in edges:
        for c in range(num_colors):
            cnf.append([-(u * num_colors + c + 1), -(v * num_colors + c + 1)])
    
    converter = SATtoCliqueConverter(cnf)
    converter.analyze_density()
    converter.write_snap_format(f"sat_generated/coloring_{num_vertices}v_{num_colors}c.txt")


if __name__ == "__main__":
    import os
    
    print("=" * 80)
    print(" " * 20 + "SAT-to-Clique Graph Generator")
    print(" " * 15 + "Generating Dense Graphs for Clique Benchmarks")
    print("=" * 80)
    
    # Create output directory
    os.makedirs("sat_generated", exist_ok=True)
    print(f"\n✓ Output directory: sat_generated/\n")
    
    # Set random seed for reproducibility
    random.seed(42)
    
    # Example 1: Simple case for testing
    print("\n" + "=" * 80)
    print("1. SIMPLE 3-SAT (8 clauses, 4 variables)")
    print("=" * 80)
    generate_simple_3sat()
    
    # Example 2: Medium random 3-SAT
    print("\n\n" + "=" * 80)
    print("2. RANDOM 3-SAT - SMALL (20 variables, 80 clauses)")
    print("=" * 80)
    generate_random_3sat(20, 80, "random_3sat_small.txt")
    
    # Example 3: Larger random 3-SAT
    print("\n\n" + "=" * 80)
    print("3. RANDOM 3-SAT - MEDIUM (50 variables, 200 clauses)")
    print("=" * 80)
    generate_random_3sat(50, 200, "random_3sat_medium.txt")
    
    # Example 4: Planted solution (satisfiable)
    print("\n\n" + "=" * 80)
    print("4. PLANTED 3-SAT (30 variables, 120 clauses, satisfiable)")
    print("=" * 80)
    generate_planted_3sat(30, 120)
    
    # Example 5: Very large (for heuristics only)
    print("\n\n" + "=" * 80)
    print("5. RANDOM 3-SAT - LARGE (100 variables, 400 clauses)")
    print("=" * 80)
    generate_random_3sat(100, 400, "random_3sat_large.txt")
    
    # Example 6: Crypto-like
    print("\n\n" + "=" * 80)
    print("6. CRYPTO-LIKE SAT (256 variables, 5000 clauses)")
    print("=" * 80)
    generate_crypto_like_sat()
    
    # Example 7: Graph coloring
    print("\n\n" + "=" * 80)
    print("7. GRAPH COLORING REDUCTION (15 vertices, 4 colors)")
    print("=" * 80)
    generate_graph_coloring_sat(15, 4, 0.5)
    
    print("\n\n" + "=" * 80)
    print(" " * 25 + "GENERATION COMPLETE!")
    print("=" * 80)
    print(f"\n✓ Generated 7 SAT-derived graphs in sat_generated/")
    print(f"✓ All graphs use SNAP edge list format")
    print(f"✓ Compatible with benchmark program")
    print(f"\nTo use these graphs:")
    print(f"  1. Update benchmark/benchmark.cpp to load from sat_generated/")
    print(f"  2. These graphs are denser and more structured than social networks")
    print(f"  3. Expect different algorithm performance characteristics\n")
