#include "degeneracy_bk.hpp"

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
    // OPTIMIZATION: Prune if current + remaining cannot beat best
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
    
    // Recurse on candidates
    for (int v : candidates) {
        // OPTIMIZATION: Check if this branch can improve best
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
    max_clique.clear();
    
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
        
        // R = {v}
        std::unordered_set<int> R;
        R.insert(v);
        
        // P = neighbors of v that come after v in ordering
        std::unordered_set<int> P;
        const auto& neighbors = g.get_neighbors(v);
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
