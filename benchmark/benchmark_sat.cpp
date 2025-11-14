#include "graph.hpp"
#include "greedy.hpp"
#include "simulated_annealing.hpp"
#include "randomized_heuristic.hpp"
#include "bron_kerbosch.hpp"
#include "tomita.hpp"
#include "degeneracy_bk.hpp"
#include "ostergard.hpp"
#include "cpu_optimized.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

struct BenchmarkResult {
    std::string algorithm_name;
    long long time_us;
    double time_seconds;
    int clique_size;
    bool valid;
    bool timed_out;
    std::string error_msg;
};

void print_result(const BenchmarkResult& r) {
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "Algorithm: " << r.algorithm_name << std::endl;
    
    if (r.timed_out) {
        std::cout << "Status: TIMED OUT" << std::endl;
    } else if (!r.error_msg.empty()) {
        std::cout << "Status: ERROR - " << r.error_msg << std::endl;
    } else {
        std::cout << "Status: SUCCESS" << std::endl;
        std::cout << "Clique Size: " << r.clique_size << std::endl;
        std::cout << "Valid: " << (r.valid ? "YES" : "NO") << std::endl;
    }
    
    std::cout << "Time (microseconds): " << r.time_us << " μs" << std::endl;
    std::cout << "Time (milliseconds): " << std::fixed << std::setprecision(2) 
              << (r.time_us / 1000.0) << " ms" << std::endl;
    std::cout << "Time (seconds): " << std::fixed << std::setprecision(4) 
              << r.time_seconds << " s" << std::endl;
    
    if (r.time_seconds >= 60) {
        std::cout << "Time (minutes): " << std::fixed << std::setprecision(2) 
                  << (r.time_seconds / 60.0) << " min" << std::endl;
    }
    
    std::cout << std::string(70, '=') << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     SAT-GENERATED GRAPH BENCHMARK - ALL ALGORITHMS FORCED         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
    
    std::string dataset_path = "../datasets/sat_generated/random_3sat_large.txt";
    if (argc > 1) {
        dataset_path = argv[1];
    }
    
    // Load graph
    std::cout << "Loading graph from: " << dataset_path << std::endl;
    Graph g;
    
    try {
        g = Graph::load_from_snap(dataset_path);
    } catch (const std::exception& e) {
        std::cerr << "Error loading graph: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "GRAPH PROPERTIES" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "Vertices: " << g.num_vertices() << std::endl;
    std::cout << "Edges: " << g.num_edges() << std::endl;
    std::cout << "Density: " << std::fixed << std::setprecision(6) << g.get_density() << std::endl;
    std::cout << "Degeneracy: " << g.get_degeneracy() << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << std::endl;
    
    std::vector<BenchmarkResult> results;
    
    // List of all algorithms
    std::vector<std::string> algorithms = {
        "Greedy",
        "SimulatedAnnealing",
        "RandomizedHeuristic",
        "BronKerbosch",
        "Tomita",
        "DegeneracyBK",
        "Ostergard",
        "CPUOptimized"
    };
    
    std::cout << "Running ALL algorithms (no size restrictions)..." << std::endl;
    std::cout << "This may take a VERY long time for large graphs!" << std::endl;
    std::cout << std::endl;
    
    for (const auto& algo_name : algorithms) {
        BenchmarkResult result;
        result.algorithm_name = algo_name;
        result.timed_out = false;
        
        std::cout << "▶ Running " << algo_name << "..." << std::endl;
        std::cout << "  Started at: " << std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()) << std::endl;
        
        std::vector<int> clique;
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            if (algo_name == "Greedy") {
                clique = GreedyClique::find_clique(g);
            } else if (algo_name == "SimulatedAnnealing") {
                SimulatedAnnealing sa(100.0, 0.995, 100000, 42);
                clique = sa.find_clique(g);
            } else if (algo_name == "RandomizedHeuristic") {
                RandomizedHeuristic rh(10, 1000, 42);
                clique = rh.find_clique(g);
            } else if (algo_name == "BronKerbosch") {
                BronKerbosch bk;
                clique = bk.find_maximum_clique(g);
            } else if (algo_name == "Tomita") {
                TomitaAlgorithm tomita;
                clique = tomita.find_maximum_clique(g);
            } else if (algo_name == "DegeneracyBK") {
                DegeneracyBK dbk;
                clique = dbk.find_maximum_clique(g);
            } else if (algo_name == "Ostergard") {
                OstergardAlgorithm ost;
                clique = ost.find_maximum_clique(g);
            } else if (algo_name == "CPUOptimized") {
                CPUOptimized cpu;
                clique = cpu.find_maximum_clique(g);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            auto duration_s = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            
            result.time_us = duration_us.count();
            result.time_seconds = duration_s.count();
            result.clique_size = clique.size();
            result.valid = g.is_clique(clique);
            
            std::cout << "  ✓ Completed!" << std::endl;
            
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            auto duration_s = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            
            result.time_us = duration_us.count();
            result.time_seconds = duration_s.count();
            result.error_msg = e.what();
            result.clique_size = 0;
            result.valid = false;
            
            std::cout << "  ✗ Error: " << e.what() << std::endl;
        }
        
        results.push_back(result);
        print_result(result);
    }
    
    // Summary
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         BENCHMARK SUMMARY                          ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
    
    std::cout << std::left << std::setw(25) << "Algorithm" 
              << std::right << std::setw(15) << "Time (s)" 
              << std::setw(12) << "Clique" 
              << std::setw(10) << "Valid" << std::endl;
    std::cout << std::string(62, '-') << std::endl;
    
    for (const auto& r : results) {
        std::cout << std::left << std::setw(25) << r.algorithm_name;
        
        if (!r.error_msg.empty()) {
            std::cout << std::right << std::setw(15) << "ERROR" 
                      << std::setw(12) << "-" 
                      << std::setw(10) << "-" << std::endl;
        } else {
            std::cout << std::right << std::setw(15) << std::fixed << std::setprecision(4) << r.time_seconds
                      << std::setw(12) << r.clique_size
                      << std::setw(10) << (r.valid ? "YES" : "NO") << std::endl;
        }
    }
    
    std::cout << std::string(62, '-') << std::endl;
    
    // Find best clique
    int max_clique_size = 0;
    std::string best_algorithm = "";
    for (const auto& r : results) {
        if (r.valid && r.clique_size > max_clique_size) {
            max_clique_size = r.clique_size;
            best_algorithm = r.algorithm_name;
        }
    }
    
    std::cout << "\nBest Result: " << best_algorithm << " with clique size " << max_clique_size << std::endl;
    
    // Save to CSV
    std::string output_csv = "../results/sat_benchmark_results.csv";
    std::ofstream csv(output_csv);
    if (csv.is_open()) {
        csv << "algorithm,time_us,time_seconds,clique_size,valid,error\n";
        for (const auto& r : results) {
            csv << r.algorithm_name << ","
                << r.time_us << ","
                << std::fixed << std::setprecision(4) << r.time_seconds << ","
                << r.clique_size << ","
                << (r.valid ? "true" : "false") << ","
                << r.error_msg << "\n";
        }
        csv.close();
        std::cout << "\nResults saved to: " << output_csv << std::endl;
    }
    
    return 0;
}
