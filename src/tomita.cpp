// tomita.cpp - Merged from tomita.hpp
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Tomita algorithm: Bron-Kerbosch with pivoting
 * 
 * Key optimization over basic Bron-Kerbosch:
 * - Choose a pivot vertex u from P ∪ X
 * - Only recurse on vertices in P \ N(u)
 * - Pivot selection: choose u that maximizes |P ∩ N(u)|
 * - This minimizes the number of recursive calls
 * 
 * Algorithm by Tomita, Tanaka, and Takahashi (2006)
 * 
 * Time complexity: O(3^(n/3)) worst case, but much faster in practice
 * Space complexity: O(n) for recursion depth
 * 
 * Typically 10-100x faster than basic Bron-Kerbosch on real graphs
 */
class TomitaAlgorithm {
public:
    /**
     * Find maximum clique using Tomita algorithm (BK with pivoting)
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Choose pivot vertex that maximizes |P ∩ N(pivot)|
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     * @return Pivot vertex
     */
    int choose_pivot(const std::unordered_set<int>& P,
                    const std::unordered_set<int>& X,
                    const Graph& g);
    
    /**
     * Recursive Tomita procedure with pivoting
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     */
    void tomita_recursive(std::unordered_set<int> R,
                         std::unordered_set<int> P,
                         std::unordered_set<int> X,
                         const Graph& g);
    
    /**
     * Compute intersection of set and vertex neighbors
     * @param s Input set
     * @param v Vertex
     * @param g Graph
     * @return s ∩ N(v)
     */
    std::unordered_set<int> intersect_with_neighbors(
        const std::unordered_set<int>& s, int v, const Graph& g);
    
    /**
     * Compute chromatic number upper bound using greedy coloring
     * @param P Set of vertices to color
     * @param g Graph
     * @return Chromatic number (upper bound)
     */
    int compute_coloring_bound(const std::unordered_set<int>& P, const Graph& g);
    
    /**
     * Find initial greedy clique for better lower bound
     * @param g Graph
     * @return Greedy clique
     */
    std::vector<int> find_greedy_clique(const Graph& g);
};



std::unordered_set<int> TomitaAlgorithm::intersect_with_neighbors(
    const std::unordered_set<int>& s, int v, const Graph& g) {
    
    std::unordered_set<int> result;
    const auto& neighbors = g.get_neighbors(v);
    
    for (int u : s) {
        if (neighbors.find(u) != neighbors.end()) {
            result.insert(u);
        }
    }
    
    return result;
}

int TomitaAlgorithm::compute_coloring_bound(const std::unordered_set<int>& P, 
                                            const Graph& g) {
    if (P.empty()) return 0;
    
    std::unordered_map<int, int> colors;
    int max_color = 0;
    
    // Order vertices by degree in P (descending)
    std::vector<int> vertices(P.begin(), P.end());
    std::sort(vertices.begin(), vertices.end(), 
              [&P, &g](int u, int v) {
                  const auto& nu = g.get_neighbors(u);
                  const auto& nv = g.get_neighbors(v);
                  int deg_u = 0, deg_v = 0;
                  for (int w : P) {
                      if (nu.find(w) != nu.end()) deg_u++;
                      if (nv.find(w) != nv.end()) deg_v++;
                  }
                  return deg_u > deg_v;
              });
    
    // Greedy sequential coloring
    for (int v : vertices) {
        std::vector<bool> used_colors(P.size() + 1, false);
        
        const auto& neighbors = g.get_neighbors(v);
        for (int u : P) {
            if (u != v && neighbors.find(u) != neighbors.end()) {
                if (colors.find(u) != colors.end()) {
                    used_colors[colors[u]] = true;
                }
            }
        }
        
        int color = 0;
        while (used_colors[color]) color++;
        
        colors[v] = color;
        max_color = std::max(max_color, color);
    }
    
    return max_color + 1;  // Chromatic number
}

std::vector<int> TomitaAlgorithm::find_greedy_clique(const Graph& g) {
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

int TomitaAlgorithm::choose_pivot(const std::unordered_set<int>& P,
                                  const std::unordered_set<int>& X,
                                  const Graph& g) {
    // Choose pivot from P ∪ X that maximizes |P ∩ N(pivot)|
    int best_pivot = -1;
    int max_intersection = -1;
    
    // Check vertices in P
    for (int v : P) {
        auto intersection = intersect_with_neighbors(P, v, g);
        if ((int)intersection.size() > max_intersection) {
            max_intersection = intersection.size();
            best_pivot = v;
        }
    }
    
    // Check vertices in X
    for (int v : X) {
        auto intersection = intersect_with_neighbors(P, v, g);
        if ((int)intersection.size() > max_intersection) {
            max_intersection = intersection.size();
            best_pivot = v;
        }
    }
    
    return best_pivot;
}

void TomitaAlgorithm::tomita_recursive(std::unordered_set<int> R,
                                       std::unordered_set<int> P,
                                       std::unordered_set<int> X,
                                       const Graph& g) {
    // OPTIMIZATION 1: Color-based upper bound pruning (tighter than |R| + |P|)
    int coloring_bound = compute_coloring_bound(P, g);
    if (R.size() + coloring_bound <= max_clique.size()) {
        return;  // Chromatic number provides tight upper bound
    }
    
    // OPTIMIZATION 2: Simple upper bound (fallback)
    if (R.size() + P.size() <= max_clique.size()) {
        return;  // Cannot find a larger clique in this branch
    }
    
    // Base case: if P and X are both empty, R is a maximal clique
    if (P.empty() && X.empty()) {
        if (R.size() > max_clique.size()) {
            max_clique.assign(R.begin(), R.end());
        }
        return;
    }
    
    // Choose pivot
    int pivot = choose_pivot(P, X, g);
    
    // Compute P \ N(pivot) - vertices to recurse on
    std::unordered_set<int> candidates;
    if (pivot != -1) {
        const auto& pivot_neighbors = g.get_neighbors(pivot);
        for (int v : P) {
            if (pivot_neighbors.find(v) == pivot_neighbors.end()) {
                candidates.insert(v);
            }
        }
    } else {
        candidates = P;
    }
    
    // OPTIMIZATION 3: Order candidates by degree (descending) for better pruning
    std::vector<int> candidates_ordered(candidates.begin(), candidates.end());
    std::sort(candidates_ordered.begin(), candidates_ordered.end(),
              [&g, &P](int u, int v) {
                  const auto& nu = g.get_neighbors(u);
                  const auto& nv = g.get_neighbors(v);
                  int deg_u = 0, deg_v = 0;
                  for (int w : P) {
                      if (nu.find(w) != nu.end()) deg_u++;
                      if (nv.find(w) != nv.end()) deg_v++;
                  }
                  return deg_u > deg_v;
              });
    
    // For each vertex in candidates (ordered)
    for (int v : candidates_ordered) {
        // OPTIMIZATION 4: Early termination check
        if (R.size() + 1 + P.size() <= max_clique.size()) {
            break;  // No point continuing
        }
        
        // Create new sets
        std::unordered_set<int> R_new = R;
        R_new.insert(v);
        
        std::unordered_set<int> P_new = intersect_with_neighbors(P, v, g);
        std::unordered_set<int> X_new = intersect_with_neighbors(X, v, g);
        
        // Recurse
        tomita_recursive(R_new, P_new, X_new, g);
        
        // Move v from P to X
        P.erase(v);
        X.insert(v);
    }
}

std::vector<int> TomitaAlgorithm::find_maximum_clique(const Graph& g) {
    // OPTIMIZATION: Seed with greedy clique for better initial lower bound
    max_clique = find_greedy_clique(g);
    
    // Initialize sets
    std::unordered_set<int> R;  // Current clique (empty initially)
    std::unordered_set<int> P;  // Candidate vertices (all vertices initially)
    std::unordered_set<int> X;  // Excluded vertices (empty initially)
    
    int n = g.num_vertices();
    for (int v = 0; v < n; v++) {
        P.insert(v);
    }
    
    // Run algorithm
    tomita_recursive(R, P, X, g);
    
    return max_clique;
}
