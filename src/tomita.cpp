#include "tomita.hpp"

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
    // OPTIMIZATION: Prune if current + remaining cannot beat best
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
    
    // For each vertex in candidates
    for (int v : candidates) {
        // OPTIMIZATION: Check if this branch can improve best
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
    max_clique.clear();
    
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
