#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
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
     * Load graph from SNAP or DIMACS edge list format
     * 
     * SNAP format: Each line contains "source_vertex target_vertex" (space/tab separated)
     *              Lines starting with '#' are treated as comments and ignored
     * 
     * DIMACS format: Lines starting with 'c' are comments
     *                Lines starting with 'p edge N M' specify vertices/edges (optional)
     *                Lines starting with 'e u v' specify edges
     * 
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

Graph::Graph(int n) : n(n), m(0) {
    adj_list.resize(n);
    adj_matrix.resize(n, std::vector<bool>(n, false));
}

Graph Graph::load_from_snap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string line;
    std::set<int> unique_vertices;
    std::vector<std::pair<int, int>> edges;
    
    // First pass: collect unique vertices and edges
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Skip comment lines (both SNAP '#' and DIMACS 'c' formats)
        if (line[0] == '#' || line[0] == 'c') {
            continue;
        }
        
        // Skip DIMACS problem line 'p edge N M' (optional, we'll auto-detect graph size)
        if (line[0] == 'p') {
            continue;
        }
        
        std::istringstream iss(line);
        
        // Check if this is DIMACS format with 'e' prefix
        char prefix;
        int u, v;
        
        if (line[0] == 'e') {
            // DIMACS format: "e u v"
            if (iss >> prefix >> u >> v) {
                edges.push_back({u, v});
                unique_vertices.insert(u);
                unique_vertices.insert(v);
            }
        } else {
            // SNAP format: "u v"
            if (iss >> u >> v) {
                edges.push_back({u, v});
                unique_vertices.insert(u);
                unique_vertices.insert(v);
            }
        }
    }
    
    file.close();
    
    if (unique_vertices.empty()) {
        throw std::runtime_error("No valid edges found in file: " + filename);
    }
    
    // Create vertex ID mapping (remap to 0-indexed contiguous range)
    std::unordered_map<int, int> vertex_map;
    int idx = 0;
    for (int v : unique_vertices) {
        vertex_map[v] = idx++;
    }
    
    // Create graph with number of unique vertices
    Graph g(unique_vertices.size());
    
    // Second pass: add edges using remapped IDs
    for (const auto& [u, v] : edges) {
        if (u != v) {  // Ignore self-loops
            g.add_edge(vertex_map[u], vertex_map[v]);
        }
    }
    
    std::cout << "Loaded graph: " << g.num_vertices() << " vertices, " 
              << g.num_edges() << " edges" << std::endl;
    
    return g;
}

void Graph::add_edge(int u, int v) {
    if (u < 0 || u >= n || v < 0 || v >= n) {
        throw std::out_of_range("Vertex ID out of range");
    }
    
    // Only add if edge doesn't exist (avoid double counting)
    if (!adj_matrix[u][v]) {
        adj_list[u].insert(v);
        adj_list[v].insert(u);
        adj_matrix[u][v] = true;
        adj_matrix[v][u] = true;
        m++;
    }
}

const std::unordered_set<int>& Graph::get_neighbors(int v) const {
    if (v < 0 || v >= n) {
        throw std::out_of_range("Vertex ID out of range");
    }
    return adj_list[v];
}

bool Graph::has_edge(int u, int v) const {
    if (u < 0 || u >= n || v < 0 || v >= n) {
        return false;
    }
    return adj_matrix[u][v];
}

int Graph::num_vertices() const {
    return n;
}

int Graph::num_edges() const {
    return m;
}

int Graph::get_degree(int v) const {
    if (v < 0 || v >= n) {
        throw std::out_of_range("Vertex ID out of range");
    }
    return adj_list[v].size();
}

std::vector<int> Graph::compute_degeneracy_ordering() const {
    std::vector<int> ordering;
    ordering.reserve(n);
    
    std::vector<int> degrees(n);
    std::vector<bool> removed(n, false);
    
    // Initialize degrees
    for (int i = 0; i < n; i++) {
        degrees[i] = adj_list[i].size();
    }
    
    // Repeatedly remove vertex with minimum degree
    for (int iter = 0; iter < n; iter++) {
        // Find vertex with minimum degree that hasn't been removed
        int min_degree = n + 1;
        int min_vertex = -1;
        
        for (int v = 0; v < n; v++) {
            if (!removed[v] && degrees[v] < min_degree) {
                min_degree = degrees[v];
                min_vertex = v;
            }
        }
        
        // Add to ordering and mark as removed
        ordering.push_back(min_vertex);
        removed[min_vertex] = true;
        
        // Update degrees of neighbors
        for (int neighbor : adj_list[min_vertex]) {
            if (!removed[neighbor]) {
                degrees[neighbor]--;
            }
        }
    }
    
    return ordering;
}

int Graph::get_degeneracy() const {
    std::vector<int> degrees(n);
    std::vector<bool> removed(n, false);
    
    // Initialize degrees
    for (int i = 0; i < n; i++) {
        degrees[i] = adj_list[i].size();
    }
    
    int degeneracy = 0;
    
    // Repeatedly remove vertex with minimum degree
    for (int iter = 0; iter < n; iter++) {
        int min_degree = n + 1;
        int min_vertex = -1;
        
        for (int v = 0; v < n; v++) {
            if (!removed[v] && degrees[v] < min_degree) {
                min_degree = degrees[v];
                min_vertex = v;
            }
        }
        
        degeneracy = std::max(degeneracy, min_degree);
        removed[min_vertex] = true;
        
        // Update degrees of neighbors
        for (int neighbor : adj_list[min_vertex]) {
            if (!removed[neighbor]) {
                degrees[neighbor]--;
            }
        }
    }
    
    return degeneracy;
}

double Graph::get_density() const {
    if (n <= 1) return 0.0;
    return (2.0 * m) / (n * (n - 1.0));
}

bool Graph::is_clique(const std::vector<int>& clique) const {
    int k = clique.size();
    for (int i = 0; i < k; i++) {
        for (int j = i + 1; j < k; j++) {
            if (!has_edge(clique[i], clique[j])) {
                return false;
            }
        }
    }
    return true;
}
