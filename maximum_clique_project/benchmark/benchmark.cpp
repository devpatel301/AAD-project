#include "graph.hpp"
#include "greedy.hpp"
#include "simulated_annealing.hpp"
#include "randomized_heuristic.hpp"
#include "bron_kerbosch.hpp"
#include "tomita.hpp"
#include "degeneracy_bk.hpp"
#include "ostergard.hpp"
#include "cpu_optimized.hpp"
// #include "gpu_optimized.cuh"  // Uncomment if CUDA is available

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

/**
 * Structure to store benchmark results
 */
struct BenchmarkResult {
    std::string dataset_name;
    std::string algorithm_name;
    long long time_us;          // Microseconds
    int clique_size;
    int num_vertices;
    int num_edges;
    double density;
    bool valid;                 // Whether result is a valid clique
    
    BenchmarkResult() : time_us(0), clique_size(0), num_vertices(0), 
                       num_edges(0), density(0.0), valid(false) {}
};

/**
 * Benchmarker class for running comprehensive algorithm comparisons
 */
class Benchmarker {
public:
    /**
     * Add a dataset to benchmark
     * @param name Descriptive name
     * @param filepath Path to edge list file
     */
    void add_dataset(const std::string& name, const std::string& filepath) {
        try {
            Graph g = Graph::load_from_snap(filepath);
            datasets.push_back({name, g});
            std::cout << "Added dataset: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error loading " << name << ": " << e.what() << std::endl;
        }
    }
    
    /**
     * Run all benchmarks and save to CSV
     * @param output_csv Output file path
     * @param timeout_seconds Maximum time per algorithm (0 = no limit)
     */
    void run_all_benchmarks(const std::string& output_csv, int timeout_seconds = 0) {
        std::cout << "\n=== Starting Benchmark Suite ===\n" << std::endl;
        
        for (const auto& [name, graph] : datasets) {
            std::cout << "\nBenchmarking dataset: " << name << std::endl;
            std::cout << "  Vertices: " << graph.num_vertices() << std::endl;
            std::cout << "  Edges: " << graph.num_edges() << std::endl;
            std::cout << "  Density: " << std::fixed << std::setprecision(6) 
                     << graph.get_density() << std::endl;
            std::cout << "  Degeneracy: " << graph.get_degeneracy() << std::endl;
            std::cout << std::endl;
            
            // Run each algorithm
            benchmark_algorithm(graph, name, "Greedy", timeout_seconds);
            benchmark_algorithm(graph, name, "SimulatedAnnealing", timeout_seconds);
            benchmark_algorithm(graph, name, "RandomizedHeuristic", timeout_seconds);
            
            // Exact algorithms - may be slow on large graphs
            if (graph.num_vertices() <= 100) {
                benchmark_algorithm(graph, name, "BronKerbosch", timeout_seconds);
                benchmark_algorithm(graph, name, "Tomita", timeout_seconds);
                benchmark_algorithm(graph, name, "DegeneracyBK", timeout_seconds);
                benchmark_algorithm(graph, name, "Ostergard", timeout_seconds);
                
                if (graph.num_vertices() <= 1024) {
                    benchmark_algorithm(graph, name, "CPUOptimized", timeout_seconds);
                }
            } else {
                std::cout << "  Skipping exact algorithms (graph too large)" << std::endl;
            }
            
            std::cout << std::string(60, '-') << std::endl;
        }
        
        // Write results to CSV
        write_results_csv(output_csv);
        
        std::cout << "\n=== Benchmark Complete ===\n" << std::endl;
        std::cout << "Results saved to: " << output_csv << std::endl;
    }
    
private:
    std::vector<std::pair<std::string, Graph>> datasets;
    std::vector<BenchmarkResult> results;
    
    /**
     * Benchmark a single algorithm on a graph
     */
    void benchmark_algorithm(const Graph& g, const std::string& dataset_name,
                           const std::string& algo_name, int timeout_seconds) {
        std::cout << "  Running " << algo_name << "... " << std::flush;
        
        BenchmarkResult result;
        result.dataset_name = dataset_name;
        result.algorithm_name = algo_name;
        result.num_vertices = g.num_vertices();
        result.num_edges = g.num_edges();
        result.density = g.get_density();
        
        std::vector<int> clique;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            // Run algorithm
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
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            result.time_us = duration.count();
            result.clique_size = clique.size();
            result.valid = g.is_clique(clique);
            
            std::cout << "Done! Clique size: " << result.clique_size 
                     << ", Time: " << (result.time_us / 1000.0) << " ms"
                     << (result.valid ? " [VALID]" : " [INVALID]") << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            result.time_us = -1;
            result.clique_size = 0;
            result.valid = false;
        }
        
        results.push_back(result);
    }
    
    /**
     * Write all results to CSV file
     */
    void write_results_csv(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open output file: " << filename << std::endl;
            return;
        }
        
        // Write header
        file << "dataset,algorithm,time_us,time_ms,clique_size,num_vertices,num_edges,density,valid\n";
        
        // Write results
        for (const auto& r : results) {
            file << r.dataset_name << ","
                 << r.algorithm_name << ","
                 << r.time_us << ","
                 << (r.time_us / 1000.0) << ","
                 << r.clique_size << ","
                 << r.num_vertices << ","
                 << r.num_edges << ","
                 << std::fixed << std::setprecision(6) << r.density << ","
                 << (r.valid ? "true" : "false") << "\n";
        }
        
        file.close();
    }
};

/**
 * Main benchmarking program
 */
int main(int argc, char* argv[]) {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════╗
║     Maximum Clique Algorithm Benchmarking Suite              ║
╚═══════════════════════════════════════════════════════════════╝
)" << std::endl;
    
    Benchmarker benchmarker;
    
    // Add datasets
    // Adjust paths as needed for your setup
    std::cout << "Loading datasets...\n" << std::endl;
    
    // Small graphs for testing all algorithms
    benchmarker.add_dataset("facebook", "../datasets/facebook_combined.txt");
    benchmarker.add_dataset("ca-GrQc", "../datasets/ca-GrQc.txt");
    
    // Medium graphs (heuristics + efficient exact algorithms)
    // benchmarker.add_dataset("ca-HepTh", "../datasets/ca-HepTh.txt");
    // benchmarker.add_dataset("email-Enron", "../datasets/email-Enron.txt");
    
    // Large graphs (heuristics only)
    // benchmarker.add_dataset("soc-Slashdot0811", "../datasets/soc-Slashdot0811.txt");
    
    // Run benchmarks
    std::string output_file = "../results/benchmark_results.csv";
    if (argc > 1) {
        output_file = argv[1];
    }
    
    benchmarker.run_all_benchmarks(output_file);
    
    std::cout << R"(
═══════════════════════════════════════════════════════════════
Dataset Sources:
- SNAP Stanford Network Dataset Collection
- http://snap.stanford.edu/data/

Instructions:
1. Download datasets from SNAP
2. Place .txt files in datasets/ directory
3. Update paths in benchmark.cpp main() if needed
4. Rebuild and run

Example datasets:
- ego-Facebook: http://snap.stanford.edu/data/ego-Facebook.html
- ca-GrQc: http://snap.stanford.edu/data/ca-GrQc.html
- ca-HepTh: http://snap.stanford.edu/data/ca-HepTh.html
- email-Enron: http://snap.stanford.edu/data/email-Enron.html
═══════════════════════════════════════════════════════════════
)" << std::endl;
    
    return 0;
}
