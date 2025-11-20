// bron_kerbosch.cpp - Merged from bron_kerbosch.hpp
#include <vector>
#include <unordered_set>
#include <algorithm>

#ifndef BRON_KERBOSCH_HPP
#define BRON_KERBOSCH_HPP

// Forward declaration
class Graph;


/**
 * Basic Bron-Kerbosch algorithm for maximum clique problem
 * 
 * Algorithm:
 * - Recursive backtracking with three sets:
 *   R: current clique being constructed
 *   P: candidate vertices that can extend R
 *   X: vertices already processed (to avoid duplicates)
 * 
 * - Base case: when P and X are empty, R is a maximal clique
 * - Recursive case: for each vertex v in P:
 *   - Add v to R
 *   - Recurse with R∪{v}, P∩N(v), X∩N(v)
 *   - Move v from P to X
 * 
 * Time complexity: O(3^(n/3)) in worst case (exponential)
 * Space complexity: O(n) for recursion depth
 * 
 * This is the basic version without pivoting optimization
 */
class BronKerbosch {
public:
    /**
     * Find maximum clique using basic Bron-Kerbosch algorithm
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Recursive Bron-Kerbosch procedure
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Input graph
     */
    void bron_kerbosch(std::unordered_set<int> R, 
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
     * Find initial greedy clique for better lower bound
     * @param g Graph
     * @return Greedy clique
     */
    std::vector<int> find_greedy_clique(const Graph& g);
};

#endif // BRON_KERBOSCH_HPP



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

std::vector<int> BronKerbosch::find_greedy_clique(const Graph& g) {
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

void BronKerbosch::bron_kerbosch(std::unordered_set<int> R,
                                 std::unordered_set<int> P,
                                 std::unordered_set<int> X,
                                 const Graph& g) {
    // PRUNING: Upper bound check - if current + all remaining can't beat best, prune
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
    bron_kerbosch(R, P, X, g);
    
    return max_clique;
}
