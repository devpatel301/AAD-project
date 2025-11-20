// maxclique_dyn.cpp - MaxCliqueDyn: Enhanced Tomita with dynamic graph coloring
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <queue>

/**
 * MaxCliqueDyn Algorithm: Tomita with Dynamic Graph Coloring
 * 
 * Key enhancements over basic Tomita:
 * 1. Dynamic Sequential Coloring: Colors vertices on-the-fly during search
 * 2. Color-based Pruning: Uses chromatic number as tight upper bound
 * 3. Reverse Color Order Processing: Process highest-colored vertices first
 * 4. Early Termination: Stop when color + |R| <= |best_clique|
 * 
 * Coloring Heuristic (Greedy Sequential):
 * - Order vertices by degree (descending)
 * - Assign minimum available color to each vertex
 * - Chromatic number χ(P) provides upper bound: |R| + χ(P)
 * 
 * Algorithm by Tomita et al., extended with dynamic coloring
 * 
 * Time complexity: O(3^(n/3)) worst case, but significantly faster in practice
 * Space complexity: O(n) for recursion depth + coloring
 * 
 * Typically 2-10x faster than standard Tomita on dense graphs
 */
class MaxCliqueDyn {
public:
    /**
     * Find maximum clique using MaxCliqueDyn algorithm
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    const Graph* graph;
    std::vector<int> max_clique;
    
    /**
     * Greedy sequential graph coloring for candidate set P
     * Returns vector of colors (indexed by vertex) and max color used
     * @param P Candidate vertex set
     * @return Pair of (vertex_colors, chromatic_number)
     */
    std::pair<std::vector<int>, int> color_graph(const std::unordered_set<int>& P);
    
    /**
     * Recursive MaxCliqueDyn procedure with dynamic coloring
     * @param R Current clique
     * @param P Candidate set
     * @param g Graph
     */
    void maxclique_dyn_recursive(std::vector<int>& R,
                                 std::unordered_set<int>& P);
    
    /**
     * Compute intersection of set P and vertex neighbors
     * @param P Input set
     * @param v Vertex
     * @return P ∩ N(v) as unordered_set
     */
    std::unordered_set<int> intersect_with_neighbors(
        const std::unordered_set<int>& P, int v);
    
    /**
     * Order vertices by degree (descending) for coloring
     * @param P Set of vertices
     * @return Vector of vertices ordered by degree
     */
    std::vector<int> order_by_degree(const std::unordered_set<int>& P);
    
    /**
     * Find initial greedy clique for better lower bound
     * @param g Graph
     * @return Greedy clique
     */
    std::vector<int> find_greedy_clique(const Graph& g);
};



std::unordered_set<int> MaxCliqueDyn::intersect_with_neighbors(
    const std::unordered_set<int>& P, int v) {
    
    std::unordered_set<int> result;
    const auto& neighbors = graph->get_neighbors(v);
    
    for (int u : P) {
        if (neighbors.find(u) != neighbors.end()) {
            result.insert(u);
        }
    }
    
    return result;
}

std::vector<int> MaxCliqueDyn::order_by_degree(const std::unordered_set<int>& P) {
    std::vector<int> vertices(P.begin(), P.end());
    
    // Sort by degree in descending order
    std::sort(vertices.begin(), vertices.end(), 
              [this, &P](int u, int v) {
                  // Count neighbors within P
                  const auto& nu = graph->get_neighbors(u);
                  const auto& nv = graph->get_neighbors(v);
                  
                  int deg_u = 0, deg_v = 0;
                  for (int w : P) {
                      if (nu.find(w) != nu.end()) deg_u++;
                      if (nv.find(w) != nv.end()) deg_v++;
                  }
                  
                  return deg_u > deg_v;  // Descending order
              });
    
    return vertices;
}

std::vector<int> MaxCliqueDyn::find_greedy_clique(const Graph& g) {
    std::vector<int> clique;
    int n = g.num_vertices();
    
    // Start with highest degree vertex
    int best_v = -1;
    int best_deg = -1;
    for (int v = 0; v < n; v++) {
        int deg = g.get_neighbors(v).size();
        if (deg > best_deg) {
            best_deg = deg;
            best_v = v;
        }
    }
    
    if (best_v == -1) return clique;
    
    clique.push_back(best_v);
    
    // Greedily add vertices that are connected to all current clique members
    std::unordered_set<int> candidates(g.get_neighbors(best_v).begin(), 
                                       g.get_neighbors(best_v).end());
    
    while (!candidates.empty()) {
        // Pick candidate with highest degree in candidates
        int next_v = -1;
        int max_deg = -1;
        
        for (int v : candidates) {
            int deg = 0;
            const auto& neighbors = g.get_neighbors(v);
            for (int u : candidates) {
                if (neighbors.find(u) != neighbors.end()) deg++;
            }
            if (deg > max_deg) {
                max_deg = deg;
                next_v = v;
            }
        }
        
        if (next_v == -1) break;
        
        clique.push_back(next_v);
        
        // Update candidates: intersect with neighbors of next_v
        std::unordered_set<int> new_candidates;
        const auto& neighbors = g.get_neighbors(next_v);
        for (int v : candidates) {
            if (v != next_v && neighbors.find(v) != neighbors.end()) {
                new_candidates.insert(v);
            }
        }
        candidates = new_candidates;
    }
    
    return clique;
}

std::pair<std::vector<int>, int> MaxCliqueDyn::color_graph(
    const std::unordered_set<int>& P) {
    
    int n = graph->num_vertices();
    std::vector<int> colors(n, -1);  // -1 means uncolored
    int max_color = 0;
    
    if (P.empty()) {
        return {colors, 0};
    }
    
    // Order vertices by degree (higher degree first)
    std::vector<int> ordered = order_by_degree(P);
    
    // Greedy sequential coloring
    for (int v : ordered) {
        // Find colors used by neighbors
        std::vector<bool> used_colors(n, false);
        
        const auto& neighbors = graph->get_neighbors(v);
        for (int u : P) {
            if (u != v && neighbors.find(u) != neighbors.end()) {
                if (colors[u] >= 0) {
                    used_colors[colors[u]] = true;
                }
            }
        }
        
        // Assign minimum available color
        int color = 0;
        while (color < n && used_colors[color]) {
            color++;
        }
        
        colors[v] = color;
        max_color = std::max(max_color, color);
    }
    
    return {colors, max_color + 1};  // +1 because colors are 0-indexed
}

void MaxCliqueDyn::maxclique_dyn_recursive(std::vector<int>& R,
                                           std::unordered_set<int>& P) {
    
    // Base case: P is empty
    if (P.empty()) {
        if (R.size() > max_clique.size()) {
            max_clique = R;
        }
        return;
    }
    
    // Dynamic coloring of P
    auto [colors, chromatic_number] = color_graph(P);
    
    // PRUNING: If current clique + chromatic number <= best, prune
    // χ(P) is an upper bound on the maximum independent set in complement
    // Therefore, |R| + χ(P) is upper bound on maximum clique
    if (R.size() + chromatic_number <= max_clique.size()) {
        return;  // Cannot improve best clique
    }
    
    // Group vertices by color
    std::vector<std::vector<int>> color_classes(chromatic_number);
    for (int v : P) {
        if (colors[v] >= 0) {
            color_classes[colors[v]].push_back(v);
        }
    }
    
    // Process vertices in reverse color order (highest color first)
    // Vertices with higher colors have better chance of being in large cliques
    for (int c = chromatic_number - 1; c >= 0; c--) {
        // OPTIMIZATION: Enhanced pruning per color class
        if (R.size() + (c + 1) <= max_clique.size()) {
            return;  // Even with all remaining color classes, cannot beat best
        }
        
        // OPTIMIZATION: Order vertices within color class by degree (descending)
        std::vector<int>& color_class = color_classes[c];
        std::sort(color_class.begin(), color_class.end(),
                  [this, &P](int u, int v) {
                      const auto& nu = graph->get_neighbors(u);
                      const auto& nv = graph->get_neighbors(v);
                      int deg_u = 0, deg_v = 0;
                      for (int w : P) {
                          if (nu.find(w) != nu.end()) deg_u++;
                          if (nv.find(w) != nv.end()) deg_v++;
                      }
                      return deg_u > deg_v;
                  });
        
        for (int v : color_class) {
            // OPTIMIZATION: Check if this branch can improve
            // Current size + remaining colors (including this one)
            if (R.size() + (c + 1) <= max_clique.size()) {
                return;  // Even with all remaining vertices, cannot beat best
            }
            
            // Add v to current clique
            R.push_back(v);
            
            // Compute new candidate set: P ∩ N(v)
            std::unordered_set<int> P_new = intersect_with_neighbors(P, v);
            
            // Recurse
            maxclique_dyn_recursive(R, P_new);
            
            // Backtrack
            R.pop_back();
            
            // Remove v from P for next iteration
            P.erase(v);
        }
    }
}

std::vector<int> MaxCliqueDyn::find_maximum_clique(const Graph& g) {
    graph = &g;
    
    // OPTIMIZATION: Seed with greedy clique for better initial lower bound
    max_clique = find_greedy_clique(g);
    
    // Initialize candidate set P with all vertices
    std::unordered_set<int> P;
    int n = g.num_vertices();
    for (int v = 0; v < n; v++) {
        P.insert(v);
    }
    
    // Initialize current clique R (empty)
    std::vector<int> R;
    
    // Run algorithm
    maxclique_dyn_recursive(R, P);
    
    return max_clique;
}
