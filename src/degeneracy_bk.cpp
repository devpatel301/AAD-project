// degeneracy_bk.cpp - Merged from degeneracy_bk.hpp
#include <vector>
#include <unordered_set>
#include <algorithm>

#ifndef DEGENERACY_BK_HPP
#define DEGENERACY_BK_HPP


/**
 * Bron-Kerbosch with degeneracy ordering
 * 
 * Key insight: Use degeneracy ordering to reduce search space
 * 
 * Algorithm:
 * 1. Compute degeneracy ordering of vertices
 * 2. For each vertex v in degeneracy order:
 *    - Let P = neighbors of v that come after v in ordering
 *    - Let X = neighbors of v that come before v in ordering
 *    - Run Tomita (BK with pivoting) on {v} ∪ P with exclusion set X
 * 3. This ensures each maximal clique is found exactly once
 * 
 * Time complexity: O(d * 3^(d/3)) where d is degeneracy
 * Space complexity: O(n)
 * 
 * Optimal for sparse graphs (where d << n)
 * Reference: Eppstein, Löffler, Strash (2010)
 */
class DegeneracyBK {
public:
    /**
     * Find maximum clique using degeneracy ordering + Tomita
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Tomita recursive procedure with pivoting
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     */
    void tomita_with_pivot(std::unordered_set<int> R,
                          std::unordered_set<int> P,
                          std::unordered_set<int> X,
                          const Graph& g);
    
    /**
     * Choose pivot vertex
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     * @return Pivot vertex
     */
    int choose_pivot(const std::unordered_set<int>& P,
                    const std::unordered_set<int>& X,
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

#endif // DEGENERACY_BK_HPP


std::unordered_set<int> DegeneracyBK::intersect_with_neighbors(
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

int DegeneracyBK::compute_coloring_bound(const std::unordered_set<int>& P, 
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

std::vector<int> DegeneracyBK::find_greedy_clique(const Graph& g) {
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

int DegeneracyBK::choose_pivot(const std::unordered_set<int>& P,
                               const std::unordered_set<int>& X,
                               const Graph& g) {
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

void DegeneracyBK::tomita_with_pivot(std::unordered_set<int> R,
                                     std::unordered_set<int> P,
                                     std::unordered_set<int> X,
                                     const Graph& g) {
    // OPTIMIZATION 1: Color-based upper bound pruning
    int coloring_bound = compute_coloring_bound(P, g);
    if (R.size() + coloring_bound <= max_clique.size()) {
        return;  // Chromatic number provides tight upper bound
    }
    
    // OPTIMIZATION 2: Simple upper bound (fallback)
    if (R.size() + P.size() <= max_clique.size()) {
        return;  // Cannot find a larger clique in this branch
    }
    
    // Base case
    if (P.empty() && X.empty()) {
        if (R.size() > max_clique.size()) {
            max_clique.assign(R.begin(), R.end());
        }
        return;
    }
    
    // Choose pivot
    int pivot = choose_pivot(P, X, g);
    
    // Compute candidates: P \ N(pivot)
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
    
    // Recurse on candidates (ordered)
    for (int v : candidates_ordered) {
        // OPTIMIZATION 4: Early termination check
        if (R.size() + 1 + P.size() <= max_clique.size()) {
            break;  // No point continuing
        }
        
        std::unordered_set<int> R_new = R;
        R_new.insert(v);
        
        std::unordered_set<int> P_new = intersect_with_neighbors(P, v, g);
        std::unordered_set<int> X_new = intersect_with_neighbors(X, v, g);
        
        tomita_with_pivot(R_new, P_new, X_new, g);
        
        P.erase(v);
        X.insert(v);
    }
}

std::vector<int> DegeneracyBK::find_maximum_clique(const Graph& g) {
    // OPTIMIZATION: Seed with greedy clique for better initial lower bound
    max_clique = find_greedy_clique(g);
    
    // Compute degeneracy ordering
    std::vector<int> ordering = g.compute_degeneracy_ordering();
    
    // Create position map for ordering
    std::vector<int> position(g.num_vertices());
    for (size_t i = 0; i < ordering.size(); i++) {
        position[ordering[i]] = i;
    }
    
    // Process vertices in degeneracy order
    for (size_t i = 0; i < ordering.size(); i++) {
        int v = ordering[i];
        
        // OPTIMIZATION: Early termination if vertex + later neighbors can't beat best
        const auto& neighbors = g.get_neighbors(v);
        int later_neighbors = 0;
        for (int u : neighbors) {
            if (position[u] > (int)i) later_neighbors++;
        }
        
        if (1 + later_neighbors <= (int)max_clique.size()) {
            continue;  // Skip this vertex, cannot improve
        }
        
        // R = {v}
        std::unordered_set<int> R;
        R.insert(v);
        
        // P = neighbors of v that come after v in ordering
        std::unordered_set<int> P;
        for (int u : neighbors) {
            if (position[u] > (int)i) {
                P.insert(u);
            }
        }
        
        // X = neighbors of v that come before v in ordering
        std::unordered_set<int> X;
        for (int u : neighbors) {
            if (position[u] < (int)i) {
                X.insert(u);
            }
        }
        
        // Run Tomita with pivoting
        tomita_with_pivot(R, P, X, g);
    }
    
    return max_clique;
}
