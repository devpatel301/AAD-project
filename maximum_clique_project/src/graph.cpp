#include "graph.hpp"

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
    int max_vertex = -1;
    std::vector<std::pair<int, int>> edges;
    
    // First pass: determine maximum vertex ID and collect edges
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            edges.push_back({u, v});
            max_vertex = std::max(max_vertex, std::max(u, v));
        }
    }
    
    file.close();
    
    if (max_vertex < 0) {
        throw std::runtime_error("No valid edges found in file: " + filename);
    }
    
    // Create graph with appropriate size
    Graph g(max_vertex + 1);
    
    // Second pass: add edges
    for (const auto& [u, v] : edges) {
        if (u != v) {  // Ignore self-loops
            g.add_edge(u, v);
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
