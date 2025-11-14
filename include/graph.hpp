#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <queue>

/**
 * Graph class for storing and manipulating undirected graphs
 * 
 * Uses dual representation:
 * - Adjacency list (vector<unordered_set<int>>): Fast neighbor iteration, O(1) edge insertion
 * - Adjacency matrix (vector<vector<bool>>): Fast edge existence check, O(1)
 * 
 * Space complexity: O(V + E) for adj_list + O(V²) for adj_matrix
 */
class Graph {
public:
    /**
     * Constructor: Creates graph with n vertices
     * @param n Number of vertices (default 0)
     */
    Graph(int n = 0);
    
    /**
     * Load graph from SNAP edge list format
     * Format: Each line contains "source_vertex target_vertex" (space/tab separated)
     * Lines starting with '#' are treated as comments and ignored
     * Automatically converts to undirected graph (adds both directions)
     * 
     * @param filename Path to edge list file
     * @return Graph object
     * @throws runtime_error if file cannot be opened
     * 
     * Time complexity: O(V + E)
     */
    static Graph load_from_snap(const std::string& filename);
    
    /**
     * Add undirected edge between vertices u and v
     * Adds both u->v and v->u
     * 
     * @param u First vertex
     * @param v Second vertex
     * 
     * Time complexity: O(1) average case
     */
    void add_edge(int u, int v);
    
    /**
     * Get neighbors of vertex v
     * @param v Vertex ID
     * @return Reference to set of neighbors
     * 
     * Time complexity: O(1)
     */
    const std::unordered_set<int>& get_neighbors(int v) const;
    
    /**
     * Check if edge exists between u and v
     * @param u First vertex
     * @param v Second vertex
     * @return true if edge exists
     * 
     * Time complexity: O(1)
     */
    bool has_edge(int u, int v) const;
    
    /**
     * Get number of vertices in graph
     * @return Number of vertices
     */
    int num_vertices() const;
    
    /**
     * Get number of edges in graph
     * @return Number of edges
     */
    int num_edges() const;
    
    /**
     * Get degree of vertex v
     * @param v Vertex ID
     * @return Degree of vertex
     * 
     * Time complexity: O(1)
     */
    int get_degree(int v) const;
    
    /**
     * Compute degeneracy ordering of vertices
     * Degeneracy d: minimum k such that every subgraph has vertex of degree ≤ k
     * 
     * Algorithm:
     * 1. Repeatedly remove vertex of minimum degree
     * 2. Order of removal is degeneracy ordering
     * 
     * @return Vector of vertex IDs in degeneracy order
     * 
     * Time complexity: O(V + E)
     */
    std::vector<int> compute_degeneracy_ordering() const;
    
    /**
     * Get degeneracy of graph
     * @return Degeneracy value
     * 
     * Time complexity: O(V + E)
     */
    int get_degeneracy() const;
    
    /**
     * Get density of graph (2*E / (V*(V-1)))
     * @return Density value between 0 and 1
     */
    double get_density() const;
    
    /**
     * Validate that given vertices form a clique
     * @param clique Vector of vertex IDs
     * @return true if all vertices are pairwise connected
     * 
     * Time complexity: O(k²) where k = clique size
     */
    bool is_clique(const std::vector<int>& clique) const;
    
private:
    int n;  // Number of vertices
    int m;  // Number of edges
    std::vector<std::unordered_set<int>> adj_list;
    std::vector<std::vector<bool>> adj_matrix;
};

#endif // GRAPH_HPP
