#include <vector>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>
#include <unordered_set>

using namespace std;

/**
 * BBMC (Branch-and-Bound Maximum Clique) Algorithm
 * 
 * Based on the algorithm by Pablo San Segundo et al.
 * Uses bitsets for efficient set operations and greedy coloring for bounds.
 * 
 * Key features:
 * - Bitset representation for fast intersection operations
 * - Greedy coloring for tight upper bounds
 * - Vertex ordering strategies (degree, neighbor degree, min-width)
 * - Branch-and-bound pruning
 * 
 * Time complexity: O(3^(n/3)) worst case, much faster in practice
 * Space complexity: O(n^2) for bitsets
 */

constexpr size_t MAX_VERTICES = 100000;  // Maximum vertices supported

class BBMC {
public:
    enum OrderingStyle {
        DEGREE_ORDER = 1,      // Sort by degree
        MIN_WIDTH_ORDER = 2,   // Minimum width ordering
        MCR_ORDER = 3          // Maximum cardinality of neighbors
    };
    
    /**
     * Constructor
     * @param g Input graph
     * @param style Vertex ordering style (default: DEGREE_ORDER)
     */
    BBMC(const Graph& g, OrderingStyle style = DEGREE_ORDER);
    
    /**
     * Find maximum clique
     * @return Vector of vertex IDs forming maximum clique
     */
    vector<int> find_maximum_clique();
    
    /**
     * Get number of nodes explored
     */
    long long get_nodes_explored() const { return nodes_explored; }
    
private:
    struct Vertex {
        int index;
        int degree;
        int neb_degree;  // Sum of neighbor degrees
        
        Vertex(int idx = 0, int deg = 0) 
            : index(idx), degree(deg), neb_degree(0) {}
        
        // For sorting by degree (descending)
        bool operator<(const Vertex& other) const {
            if (degree != other.degree) {
                return degree > other.degree;  // Descending
            }
            return neb_degree > other.neb_degree;
        }
    };
    
    const Graph& graph;
    int n;  // Number of vertices
    OrderingStyle ordering_style;
    
    // Bitset representations
    vector<bitset<MAX_VERTICES>> N;     // Neighborhoods
    vector<bitset<MAX_VERTICES>> invN;  // Inverse neighborhoods
    vector<Vertex> V;  // Vertex mapping
    
    // Search state
    vector<int> best_clique;
    int max_size;
    long long nodes_explored;
    
    // Core algorithm
    void bb_max_clique(bitset<MAX_VERTICES>& C, bitset<MAX_VERTICES>& P);
    
    // Coloring for bounds
    void bb_colour(const bitset<MAX_VERTICES>& P, 
                   vector<int>& U, 
                   vector<int>& colour);
    
    // Vertex ordering
    void order_vertices();
    void min_width_order(vector<Vertex>& vertices);
    
    // Solution management
    void save_solution(const bitset<MAX_VERTICES>& C);
    
    // Utility
    int count_bits(const bitset<MAX_VERTICES>& bs) const;
};

BBMC::BBMC(const Graph& g, OrderingStyle style) 
    : graph(g), n(g.num_vertices()), ordering_style(style), 
      max_size(0), nodes_explored(0) {
    
    if (n > MAX_VERTICES) {
        throw runtime_error("Graph too large for BBMC (max " + 
                          to_string(MAX_VERTICES) + " vertices)");
    }
    
    N.resize(n);
    invN.resize(n);
    V.resize(n);
}

vector<int> BBMC::find_maximum_clique() {
    nodes_explored = 0;
    max_size = 0;
    best_clique.clear();
    
    // Initialize vertex data
    for (int i = 0; i < n; i++) {
        V[i] = Vertex(i, graph.get_degree(i));
        N[i].reset();
        invN[i].reset();
    }
    
    // Order vertices
    order_vertices();
    
    // Initialize search
    bitset<MAX_VERTICES> C;  // Current clique
    bitset<MAX_VERTICES> P;  // Candidate set
    
    C.reset();
    for (int i = 0; i < n; i++) {
        P.set(i);
    }
    
    // Run search
    bb_max_clique(C, P);
    
    return best_clique;
}

void BBMC::bb_max_clique(bitset<MAX_VERTICES>& C, bitset<MAX_VERTICES>& P) {
    nodes_explored++;
    
    int m = P.count();
    if (m == 0) {
        if (C.count() > max_size) {
            save_solution(C);
        }
        return;
    }
    
    // Color vertices to get upper bound
    vector<int> U(m);
    vector<int> colour(m);
    bb_colour(P, U, colour);
    
    // Process vertices in reverse color order (best first)
    for (int i = m - 1; i >= 0; i--) {
        // Prune: if color + current clique size <= best known, stop
        if (colour[i] + (int)C.count() <= max_size) {
            return;
        }
        
        int v = U[i];
        
        // Create new candidate set: P ∩ N(v)
        bitset<MAX_VERTICES> newP = P;
        newP &= N[v];
        
        // Add v to clique
        C.set(v);
        
        // Check if we have a maximal clique
        if (newP.none()) {
            if (C.count() > max_size) {
                save_solution(C);
            }
        } else {
            // Recurse
            bb_max_clique(C, newP);
        }
        
        // Backtrack
        P.reset(v);
        C.reset(v);
    }
}

void BBMC::bb_colour(const bitset<MAX_VERTICES>& P, 
                     vector<int>& U, 
                     vector<int>& colour) {
    bitset<MAX_VERTICES> copyP = P;
    int colour_class = 0;
    int i = 0;
    
    // Greedy coloring
    while (copyP.any()) {
        colour_class++;
        bitset<MAX_VERTICES> Q = copyP;
        
        while (Q.any()) {
            // Find first set bit
            int v = -1;
            for (int j = 0; j < n; j++) {
                if (Q.test(j)) {
                    v = j;
                    break;
                }
            }
            
            if (v == -1) break;
            
            // Remove v from both sets
            copyP.reset(v);
            Q.reset(v);
            
            // Remove neighbors of v from Q
            Q &= invN[v];
            
            // Record vertex and its color
            U[i] = v;
            colour[i] = colour_class;
            i++;
        }
    }
}

void BBMC::order_vertices() {
    // Calculate neighbor degrees
    for (int i = 0; i < n; i++) {
        V[i].neb_degree = 0;
        for (int j = 0; j < n; j++) {
            if (graph.has_edge(V[i].index, j)) {
                V[i].neb_degree += graph.get_degree(j);
            }
        }
    }
    
    // Apply ordering strategy
    if (ordering_style == DEGREE_ORDER) {
        sort(V.begin(), V.end());
    } else if (ordering_style == MIN_WIDTH_ORDER) {
        min_width_order(V);
    } else if (ordering_style == MCR_ORDER) {
        // Sort by maximum cardinality ratio (neighbor degree)
        sort(V.begin(), V.end(), [](const Vertex& a, const Vertex& b) {
            if (a.neb_degree != b.neb_degree) {
                return a.neb_degree > b.neb_degree;
            }
            return a.degree > b.degree;
        });
    }
    
    // Build bitset representations based on new ordering
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int u = V[i].index;
            int v = V[j].index;
            
            if (graph.has_edge(u, v)) {
                N[i].set(j);
                invN[i].reset(j);
            } else {
                N[i].reset(j);
                invN[i].set(j);
            }
        }
    }
}

void BBMC::min_width_order(vector<Vertex>& vertices) {
    // Minimum width ordering (similar to degeneracy ordering)
    vector<bool> ordered(n, false);
    vector<Vertex> result;
    vector<int> current_degree(n);
    
    // Initialize current degrees
    for (int i = 0; i < n; i++) {
        current_degree[i] = vertices[i].degree;
    }
    
    // Greedily select vertices with minimum degree
    for (int iter = 0; iter < n; iter++) {
        int min_deg = n + 1;
        int min_idx = -1;
        
        for (int i = 0; i < n; i++) {
            if (!ordered[i] && current_degree[i] < min_deg) {
                min_deg = current_degree[i];
                min_idx = i;
            }
        }
        
        if (min_idx == -1) break;
        
        result.push_back(vertices[min_idx]);
        ordered[min_idx] = true;
        
        // Update degrees of neighbors
        int u = vertices[min_idx].index;
        for (int i = 0; i < n; i++) {
            if (!ordered[i] && graph.has_edge(u, vertices[i].index)) {
                current_degree[i]--;
            }
        }
    }
    
    vertices = result;
}

void BBMC::save_solution(const bitset<MAX_VERTICES>& C) {
    best_clique.clear();
    
    for (int i = 0; i < n; i++) {
        if (C.test(i)) {
            best_clique.push_back(V[i].index);
        }
    }
    
    max_size = best_clique.size();
}

int BBMC::count_bits(const bitset<MAX_VERTICES>& bs) const {
    return bs.count();
}
