// greedy.cpp - Merged from greedy.hpp
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <string>

/**
 * Greedy algorithm for maximum clique problem
 * 
 * Algorithm:
 * 1. Sort vertices by degree in descending order
 * 2. Initialize empty clique
 * 3. For each vertex in sorted order:
 *    - If vertex is connected to all vertices in current clique, add it
 * 4. Return the constructed clique
 * 
 * Time complexity: O(V² + E) - O(V log V) for sorting, O(V²) for clique construction
 * Space complexity: O(V)
 * 
 * Note: This is a fast heuristic but doesn't guarantee optimal solution
 */
class GreedyClique {
public:
    /**
     * Find a maximal clique using greedy approach
     * @param g Input graph
     * @return Vector of vertex IDs forming the clique
     */
    static std::vector<int> find_clique(const Graph& g);
};


std::vector<int> GreedyClique::find_clique(const Graph& g) {
    int n = g.num_vertices();
    std::vector<int> clique;
    
    if (n == 0) return clique;
    
    // Create list of vertices with their degrees
    std::vector<std::pair<int, int>> vertices_by_degree;
    vertices_by_degree.reserve(n);
    
    for (int v = 0; v < n; v++) {
        vertices_by_degree.push_back({g.get_degree(v), v});
    }
    
    // Sort by degree in descending order
    std::sort(vertices_by_degree.begin(), vertices_by_degree.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return a.first > b.first;
              });
    
    // Greedily construct clique
    for (const auto& [degree, v] : vertices_by_degree) {
        // Check if v is connected to all vertices in current clique
        bool connected_to_all = true;
        for (int u : clique) {
            if (!g.has_edge(v, u)) {
                connected_to_all = false;
                break;
            }
        }
        
        // If connected to all, add to clique
        if (connected_to_all) {
            clique.push_back(v);
        }
    }
    
    return clique;
}
