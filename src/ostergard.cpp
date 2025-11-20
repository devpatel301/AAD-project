// ostergard.cpp - Merged from ostergard.hpp
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <string>

/**
 * Östergård's algorithm for maximum clique
 * 
 * Branch-and-bound algorithm with vertex coloring for upper bounds
 * 
 * Key ideas:
 * 1. Order vertices by decreasing degree
 * 2. Use greedy graph coloring to compute upper bound on clique size
 * 3. Prune branches when: current_size + upper_bound ≤ best_known_size
 * 4. Branch on vertices in order, maintaining candidate set
 * 
 * Coloring bound: If vertices can be colored with k colors, 
 * then maximum clique size in that subgraph is at most k
 * (because clique vertices must all have different colors)
 * 
 * Time complexity: Exponential, but with effective pruning
 * Space complexity: O(n) for recursion
 * 
 * Reference: Östergård (2002) "A fast algorithm for the maximum clique problem"
 */
class OstergardAlgorithm {
public:
    /**
     * Find maximum clique using Östergård's algorithm
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Compute upper bound on clique size using greedy coloring
     * Colors vertices greedily and returns number of colors used
     * 
     * @param candidates Candidate vertices
     * @param g Graph
     * @return Upper bound (number of colors)
     */
    int color_bound(const std::vector<int>& candidates, const Graph& g);
    
    /**
     * Branch-and-bound recursive procedure
     * @param current Current clique being constructed
     * @param candidates Remaining candidate vertices
     * @param g Graph
     */
    void branch_and_bound(std::vector<int> current,
                         std::vector<int> candidates,
                         const Graph& g);
};



int OstergardAlgorithm::color_bound(const std::vector<int>& candidates, const Graph& g) {
    if (candidates.empty()) return 0;
    
    // Greedy coloring to get upper bound
    std::unordered_map<int, int> vertex_color;
    int num_colors = 0;
    
    for (int v : candidates) {
        // Find smallest color not used by neighbors
        std::vector<bool> used_colors(candidates.size() + 1, false);
        
        for (int u : candidates) {
            if (u != v && g.has_edge(v, u) && vertex_color.find(u) != vertex_color.end()) {
                used_colors[vertex_color[u]] = true;
            }
        }
        
        // Assign smallest available color
        int color = 0;
        while (used_colors[color]) {
            color++;
        }
        
        vertex_color[v] = color;
        num_colors = std::max(num_colors, color + 1);
    }
    
    return num_colors;
}

void OstergardAlgorithm::branch_and_bound(std::vector<int> current,
                                          std::vector<int> candidates,
                                          const Graph& g) {
    // Update best solution if current is better
    if (current.size() > max_clique.size()) {
        max_clique = current;
    }
    
    // Base case: no more candidates
    if (candidates.empty()) {
        return;
    }
    
    // Compute upper bound using coloring
    int upper_bound = color_bound(candidates, g);
    
    // Pruning: if current + upper_bound can't beat best, skip this branch
    if (current.size() + upper_bound <= max_clique.size()) {
        return;
    }
    
    // Try adding each candidate vertex
    while (!candidates.empty()) {
        // Take last vertex (highest degree due to initial sorting)
        int v = candidates.back();
        candidates.pop_back();
        
        // Pruning: if current + remaining + 1 can't beat best, stop
        if (current.size() + candidates.size() + 1 <= max_clique.size()) {
            break;
        }
        
        // Create new current clique with v
        std::vector<int> new_current = current;
        new_current.push_back(v);
        
        // Create new candidate set: vertices connected to v and all in current
        std::vector<int> new_candidates;
        for (int u : candidates) {
            bool connected_to_all = g.has_edge(v, u);
            if (connected_to_all) {
                for (int w : current) {
                    if (!g.has_edge(u, w)) {
                        connected_to_all = false;
                        break;
                    }
                }
            }
            
            if (connected_to_all) {
                new_candidates.push_back(u);
            }
        }
        
        // Recurse
        branch_and_bound(new_current, new_candidates, g);
    }
}

std::vector<int> OstergardAlgorithm::find_maximum_clique(const Graph& g) {
    max_clique.clear();
    
    int n = g.num_vertices();
    
    // Create initial candidate list sorted by degree (descending)
    std::vector<std::pair<int, int>> vertices_by_degree;
    for (int v = 0; v < n; v++) {
        vertices_by_degree.push_back({g.get_degree(v), v});
    }
    
    std::sort(vertices_by_degree.begin(), vertices_by_degree.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    std::vector<int> candidates;
    for (const auto& [degree, v] : vertices_by_degree) {
        candidates.push_back(v);
    }
    
    // Start branch and bound
    std::vector<int> current;
    branch_and_bound(current, candidates, g);
    
    return max_clique;
}
