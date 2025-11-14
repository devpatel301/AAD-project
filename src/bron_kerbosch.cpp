#include "bron_kerbosch.hpp"

std::unordered_set<int> BronKerbosch::intersect_with_neighbors(
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

void BronKerbosch::bron_kerbosch(std::unordered_set<int> R,
                                 std::unordered_set<int> P,
                                 std::unordered_set<int> X,
                                 const Graph& g) {
    // OPTIMIZATION: Prune if current + remaining cannot beat best
    // Upper bound: |R| + |P| (assuming all of P can be added)
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
    
    // Make a copy of P to iterate over (since we'll modify P)
    std::unordered_set<int> P_copy = P;
    
    // For each vertex in P
    for (int v : P_copy) {
        // OPTIMIZATION: Check if this branch can improve best
        if (R.size() + 1 + P.size() <= max_clique.size()) {
            break;  // No point continuing, remaining vertices can't help
        }
        
        // Create new sets
        std::unordered_set<int> R_new = R;
        R_new.insert(v);
        
        std::unordered_set<int> P_new = intersect_with_neighbors(P, v, g);
        std::unordered_set<int> X_new = intersect_with_neighbors(X, v, g);
        
        // Recurse
        bron_kerbosch(R_new, P_new, X_new, g);
        
        // Move v from P to X
        P.erase(v);
        X.insert(v);
    }
}

std::vector<int> BronKerbosch::find_maximum_clique(const Graph& g) {
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
    bron_kerbosch(R, P, X, g);
    
    return max_clique;
}
