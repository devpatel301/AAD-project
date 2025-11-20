#include "src/graph.cpp"
#include "src/greedy.cpp"
#include "src/randomized_heuristic.cpp"
#include "src/simulated_annealing.cpp"
#include "src/ostergard.cpp"
#include "src/bbmc.cpp"
#include "src/degeneracy_bk.cpp"
#include "src/tomita.cpp"
#include "src/bron_kerbosch.cpp"
#include "src/cpu_optimized.cpp"
#include "src/maxclique_dyn.cpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <sys/resource.h>

// Memory measurement utilities
size_t get_memory_usage_kb() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;  // In KB on Linux, bytes on macOS
}

// Graph statistics computation
struct GraphStats {
    int num_vertices;
    int num_edges;
    double density;
    int max_degree;
    double avg_degree;
    int degeneracy;
    
    void compute(const Graph& g) {
        num_vertices = g.num_vertices();
        num_edges = g.num_edges();
        density = g.get_density();
        
        // Compute max and avg degree
        max_degree = 0;
        long long total_degree = 0;
        for (int v = 0; v < num_vertices; v++) {
            int deg = g.get_degree(v);
            max_degree = std::max(max_degree, deg);
            total_degree += deg;
        }
        avg_degree = num_vertices > 0 ? (double)total_degree / num_vertices : 0.0;
        
        // Compute degeneracy
        degeneracy = g.get_degeneracy();
    }
};

// Benchmark result structure
struct BenchmarkResult {
    std::string algorithm;
    int clique_size;
    double time_seconds;
    size_t memory_kb;
    bool success;
    std::string error;
};

// Helper functions for algorithms with different interfaces
BenchmarkResult run_greedy(const Graph& g) {
    BenchmarkResult result;
    result.algorithm = "Greedy";
    result.success = false;
    size_t mem_before = get_memory_usage_kb();
    auto start = std::chrono::high_resolution_clock::now();
    try {
        std::vector<int> clique = GreedyClique::find_clique(g);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        size_t mem_after = get_memory_usage_kb();
        if (g.is_clique(clique)) {
            result.clique_size = clique.size();
            result.time_seconds = elapsed.count();
            result.memory_kb = mem_after - mem_before;
            result.success = true;
        }
    } catch (...) { result.error = "Exception"; }
    return result;
}

BenchmarkResult run_randomized(const Graph& g) {
    BenchmarkResult result;
    result.algorithm = "Randomized";
    result.success = false;
    size_t mem_before = get_memory_usage_kb();
    auto start = std::chrono::high_resolution_clock::now();
    try {
        RandomizedHeuristic algo;
        std::vector<int> clique = algo.find_clique(g);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        size_t mem_after = get_memory_usage_kb();
        if (g.is_clique(clique)) {
            result.clique_size = clique.size();
            result.time_seconds = elapsed.count();
            result.memory_kb = mem_after - mem_before;
            result.success = true;
        }
    } catch (...) { result.error = "Exception"; }
    return result;
}

BenchmarkResult run_simulated_annealing(const Graph& g) {
    BenchmarkResult result;
    result.algorithm = "Simulated Annealing";
    result.success = false;
    size_t mem_before = get_memory_usage_kb();
    auto start = std::chrono::high_resolution_clock::now();
    try {
        SimulatedAnnealing algo;
        std::vector<int> clique = algo.find_clique(g);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        size_t mem_after = get_memory_usage_kb();
        if (g.is_clique(clique)) {
            result.clique_size = clique.size();
            result.time_seconds = elapsed.count();
            result.memory_kb = mem_after - mem_before;
            result.success = true;
        }
    } catch (...) { result.error = "Exception"; }
    return result;
}

BenchmarkResult run_bbmc(const Graph& g) {
    BenchmarkResult result;
    result.algorithm = "BBMC";
    result.success = false;
    size_t mem_before = get_memory_usage_kb();
    auto start = std::chrono::high_resolution_clock::now();
    try {
        BBMC algo(g, BBMC::DEGREE_ORDER);
        std::vector<int> clique = algo.find_maximum_clique();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        size_t mem_after = get_memory_usage_kb();
        if (g.is_clique(clique)) {
            result.clique_size = clique.size();
            result.time_seconds = elapsed.count();
            result.memory_kb = mem_after - mem_before;
            result.success = true;
        }
    } catch (...) { result.error = "Exception"; }
    return result;
}

// Run single algorithm with timeout and memory tracking
template<typename AlgoClass>
BenchmarkResult run_algorithm(const Graph& g, const std::string& algo_name) {
    BenchmarkResult result;
    result.algorithm = algo_name;
    result.success = false;
    
    size_t mem_before = get_memory_usage_kb();
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        AlgoClass algo;
        std::vector<int> clique = algo.find_maximum_clique(g);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        size_t mem_after = get_memory_usage_kb();
        
        if (g.is_clique(clique)) {
            result.clique_size = clique.size();
            result.time_seconds = elapsed.count();
            result.memory_kb = mem_after - mem_before;
            result.success = true;
        } else {
            result.error = "Invalid clique returned";
        }
    } catch (const std::exception& e) {
        result.error = std::string("Exception: ") + e.what();
    } catch (...) {
        result.error = "Unknown exception";
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <graph_file>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    std::string dataset_name = filename.substr(filename.find_last_of("/\\") + 1);
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n";
    std::cout << "========================================================================================================\n";
    std::cout << "  COMPREHENSIVE MAXIMUM CLIQUE BENCHMARK\n";
    std::cout << "========================================================================================================\n";
    std::cout << "Dataset: " << dataset_name << "\n";
    std::cout << "========================================================================================================\n\n";
    
    // Load graph
    std::cout << "Loading graph...\n";
    Graph g;
    try {
        g = Graph::load_from_snap(filename);
    } catch (const std::exception& e) {
        std::cerr << "Error loading graph: " << e.what() << std::endl;
        return 1;
    }
    
    // Compute graph statistics
    GraphStats stats;
    stats.compute(g);
    
    std::cout << "\nGRAPH STATISTICS:\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    std::cout << "  Vertices:      " << std::setw(10) << stats.num_vertices << "\n";
    std::cout << "  Edges:         " << std::setw(10) << stats.num_edges << "\n";
    std::cout << "  Density:       " << std::setw(10) << std::fixed << std::setprecision(4) << (stats.density * 100) << " %\n";
    std::cout << "  Max Degree:    " << std::setw(10) << stats.max_degree << "\n";
    std::cout << "  Avg Degree:    " << std::setw(10) << std::fixed << std::setprecision(2) << stats.avg_degree << "\n";
    std::cout << "  Degeneracy:    " << std::setw(10) << stats.degeneracy << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n\n";
    
    // Run all algorithms
    std::vector<BenchmarkResult> results;
    
    std::cout << "RUNNING ALGORITHMS:\n";
    std::cout << "========================================================================================================\n\n";
    
    // 1. Greedy
    std::cout << "[1/11] Greedy Heuristic...                    ";
    std::cout.flush();
    auto r1 = run_greedy(g);
    results.push_back(r1);
    if (r1.success) {
        std::cout << "✓ Size: " << std::setw(3) << r1.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r1.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r1.error << "\n";
    }
    
    // 2. Randomized Heuristic
    std::cout << "[2/11] Randomized Heuristic...                ";
    std::cout.flush();
    auto r2 = run_randomized(g);
    results.push_back(r2);
    if (r2.success) {
        std::cout << "✓ Size: " << std::setw(3) << r2.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r2.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r2.error << "\n";
    }
    
    // 3. Simulated Annealing
    std::cout << "[3/11] Simulated Annealing...                 ";
    std::cout.flush();
    auto r3 = run_simulated_annealing(g);
    results.push_back(r3);
    if (r3.success) {
        std::cout << "✓ Size: " << std::setw(3) << r3.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r3.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r3.error << "\n";
    }
    
    // 4. Bron-Kerbosch (skip if too large or too dense)
    if (stats.num_vertices <= 1000 && stats.density <= 0.5) {
        std::cout << "[4/11] Bron-Kerbosch (Vanilla)...             ";
        std::cout.flush();
        auto r4 = run_algorithm<BronKerbosch>(g, "Bron-Kerbosch");
        results.push_back(r4);
        if (r4.success) {
            std::cout << "✓ Size: " << std::setw(3) << r4.clique_size 
                      << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r4.time_seconds << " s\n";
        } else {
            std::cout << "✗ " << r4.error << "\n";
        }
    } else {
        std::string reason = stats.num_vertices > 1000 ? ">1000 vertices" : "density > 0.5";
        std::cout << "[4/11] Bron-Kerbosch (Vanilla)...             ⊘ SKIPPED (" << reason << ")\n";
        results.push_back({"Bron-Kerbosch", 0, 0.0, 0, false, "Skipped: " + reason});
    }
    
    // 5. Tomita
    std::cout << "[5/11] Tomita (BK with Pivoting)...           ";
    std::cout.flush();
    auto r5 = run_algorithm<TomitaAlgorithm>(g, "Tomita");
    results.push_back(r5);
    if (r5.success) {
        std::cout << "✓ Size: " << std::setw(3) << r5.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r5.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r5.error << "\n";
    }
    
    // 6. Degeneracy BK
    std::cout << "[6/11] Degeneracy Bron-Kerbosch...            ";
    std::cout.flush();
    auto r6 = run_algorithm<DegeneracyBK>(g, "Degeneracy BK");
    results.push_back(r6);
    if (r6.success) {
        std::cout << "✓ Size: " << std::setw(3) << r6.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r6.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r6.error << "\n";
    }
    
    // 7. Östergård
    std::cout << "[7/11] Östergård...                           ";
    std::cout.flush();
    auto r7 = run_algorithm<OstergardAlgorithm>(g, "Ostergard");
    results.push_back(r7);
    if (r7.success) {
        std::cout << "✓ Size: " << std::setw(3) << r7.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r7.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r7.error << "\n";
    }
    
    // 8. BBMC
    std::cout << "[8/11] BBMC...                                ";
    std::cout.flush();
    auto r8 = run_bbmc(g);
    results.push_back(r8);
    if (r8.success) {
        std::cout << "✓ Size: " << std::setw(3) << r8.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r8.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r8.error << "\n";
    }
    
    // 9. CPU Optimized (skip if >1000 vertices OR density >0.5 OR >8192 vertices)
    if (stats.num_vertices > 1000 || stats.density > 0.5) {
        std::cout << "[9/11] CPU Optimized...                       ⊘ SKIPPED (vertices > 1000 or density > 0.5)\n";
        results.push_back({"CPU Optimized", 0, 0.0, 0, false, "Skipped: graph too large/dense"});
    } else if (stats.num_vertices <= 8192) {
        std::cout << "[9/11] CPU Optimized...                       ";
        std::cout.flush();
        auto r9 = run_algorithm<CPUOptimized>(g, "CPU Optimized");
        results.push_back(r9);
        if (r9.success) {
            std::cout << "✓ Size: " << std::setw(3) << r9.clique_size 
                      << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r9.time_seconds << " s\n";
        } else {
            std::cout << "✗ " << r9.error << "\n";
        }
    } else {
        std::cout << "[9/11] CPU Optimized...                       ⊘ SKIPPED (>8192 vertices)\n";
        results.push_back({"CPU Optimized", 0, 0.0, 0, false, "Skipped: too large"});
    }
    
    // 10. MaxCliqueDyn
    std::cout << "[10/11] MaxCliqueDyn (Tomita + Coloring)...   ";
    std::cout.flush();
    auto r10 = run_algorithm<MaxCliqueDyn>(g, "MaxCliqueDyn");
    results.push_back(r10);
    if (r10.success) {
        std::cout << "✓ Size: " << std::setw(3) << r10.clique_size 
                  << ", Time: " << std::setw(10) << std::fixed << std::setprecision(6) << r10.time_seconds << " s\n";
    } else {
        std::cout << "✗ " << r10.error << "\n";
    }
    
    std::cout << "\n========================================================================================================\n";
    std::cout << "BENCHMARK COMPLETE\n";
    std::cout << "========================================================================================================\n\n";
    
    // Generate CSV output
    std::string csv_filename = "benchmark_detailed_" + dataset_name + ".csv";
    std::ofstream csv(csv_filename);
    
    csv << "Dataset,Vertices,Edges,Density,MaxDegree,AvgDegree,Degeneracy,";
    csv << "Algorithm,CliqueSize,Time(s),Memory(KB),Success\n";
    
    for (const auto& r : results) {
        csv << dataset_name << ","
            << stats.num_vertices << ","
            << stats.num_edges << ","
            << std::fixed << std::setprecision(6) << stats.density << ","
            << stats.max_degree << ","
            << std::fixed << std::setprecision(2) << stats.avg_degree << ","
            << stats.degeneracy << ","
            << r.algorithm << ",";
        
        if (r.success) {
            csv << r.clique_size << ","
                << std::fixed << std::setprecision(6) << r.time_seconds << ","
                << r.memory_kb << ","
                << "true\n";
        } else {
            csv << "N/A,N/A,N/A,false\n";
        }
    }
    
    csv.close();
    std::cout << "CSV file saved: " << csv_filename << "\n\n";
    
    // Print summary table
    std::cout << "RESULTS SUMMARY:\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(30) << "Algorithm" 
              << std::right << std::setw(12) << "Clique Size" 
              << std::setw(15) << "Time (s)" 
              << std::setw(15) << "Memory (KB)" << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    
    for (const auto& r : results) {
        std::cout << std::left << std::setw(30) << r.algorithm;
        if (r.success) {
            std::cout << std::right << std::setw(12) << r.clique_size
                      << std::setw(15) << std::fixed << std::setprecision(6) << r.time_seconds
                      << std::setw(15) << r.memory_kb << "\n";
        } else {
            std::cout << std::right << std::setw(12) << "FAILED"
                      << std::setw(15) << "N/A"
                      << std::setw(15) << "N/A" << "\n";
        }
    }
    std::cout << "--------------------------------------------------------------------------------------------------------\n";
    
    return 0;
}
