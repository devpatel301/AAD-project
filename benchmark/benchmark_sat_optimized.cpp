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

using namespace std;

struct BenchmarkResult {
    string algorithm_name;
    long long time_us;
    double time_seconds;
    int clique_size;
    bool valid;
    bool timed_out;
    string error_msg;
};

void print_result(const BenchmarkResult& r) {
    cout << string(70, '=') << endl;
    cout << "Algorithm: " << r.algorithm_name << endl;
    
    if (r.timed_out) {
        cout << "Status: TIMED OUT" << endl;
    } else if (!r.error_msg.empty()) {
        cout << "Status: ERROR - " << r.error_msg << endl;
    } else {
        cout << "Status: SUCCESS" << endl;
        cout << "Clique Size: " << r.clique_size << endl;
        cout << "Valid: " << (r.valid ? "YES" : "NO") << endl;
    }
    
    cout << "Time (microseconds): " << r.time_us << " μs" << endl;
    cout << "Time (milliseconds): " << fixed << setprecision(2) 
              << (r.time_us / 1000.0) << " ms" << endl;
    cout << "Time (seconds): " << fixed << setprecision(4) 
              << r.time_seconds << " s" << endl;
    
    if (r.time_seconds >= 60) {
        cout << "Time (minutes): " << fixed << setprecision(2) 
                  << (r.time_seconds / 60.0) << " min" << endl;
    }
    
    cout << string(70, '=') << endl << endl;
}

int main(int argc, char* argv[]) {
    // C++ I/O optimizations
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    
    cout << "\n";
    cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "║          SAT-GENERATED GRAPH BENCHMARK - OPTIMIZED                 ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    cout << endl;
    
    string dataset_path = "datasets/sat_generated/random_3sat_large.txt";
    if (argc > 1) {
        dataset_path = argv[1];
    }
    
    // Load graph
    cout << "Loading graph from: " << dataset_path << endl;
    Graph g;
    
    try {
        g = Graph::load_from_snap(dataset_path);
    } catch (const exception& e) {
        cerr << "Error loading graph: " << e.what() << endl;
        return 1;
    }
    
    cout << "\n" << string(70, '=') << endl;
    cout << "GRAPH PROPERTIES" << endl;
    cout << string(70, '=') << endl;
    cout << "Vertices: " << g.num_vertices() << endl;
    cout << "Edges: " << g.num_edges() << endl;
    cout << "Density: " << fixed << setprecision(6) << g.get_density() << endl;
    cout << "Degeneracy: " << g.get_degeneracy() << endl;
    cout << string(70, '=') << endl;
    cout << endl;
    
    vector<BenchmarkResult> results;
    
    // List of algorithms in optimized order:
    // 1. Heuristics first (fast)
    // 2. Optimized exact algorithms (Degeneracy BK, Tomita)
    // 3. Basic exact algorithms (Bron-Kerbosch, Ostergard)
    // 4. Specialized (CPU Optimized)
    vector<string> algorithms = {
        "Greedy",
        "RandomizedHeuristic",
        "SimulatedAnnealing",
        "DegeneracyBK",
        "Tomita",
        "CPUOptimized",
        "BronKerbosch",
        "Ostergard"
    };
    
    cout << "Running ALL algorithms (no hardcoded optimizations)..." << endl;
    cout << "Order: Heuristics → Optimized Exact → Basic Exact" << endl;
    cout << "Warning: This may take hours for dense graphs!" << endl;
    cout << endl;
    
    for (const auto& algo_name : algorithms) {
        BenchmarkResult result;
        result.algorithm_name = algo_name;
        result.timed_out = false;
        
        cout << "▶ Running " << algo_name << "..." << endl;
        
        auto now = chrono::system_clock::now();
        auto now_t = chrono::system_clock::to_time_t(now);
        cout << "  Started at: " << ctime(&now_t);
        
        vector<int> clique;
        auto start = chrono::high_resolution_clock::now();
        
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
                if (g.num_vertices() > 1024) {
                    throw runtime_error("Graph too large for bitset optimization (max 1024 vertices)");
                }
                CPUOptimized cpu;
                clique = cpu.find_maximum_clique(g);
            }
            
            auto end = chrono::high_resolution_clock::now();
            auto duration_us = chrono::duration_cast<chrono::microseconds>(end - start);
            auto duration_s = chrono::duration_cast<chrono::duration<double>>(end - start);
            
            result.time_us = duration_us.count();
            result.time_seconds = duration_s.count();
            result.clique_size = clique.size();
            result.valid = g.is_clique(clique);
            
            cout << "  ✓ Completed!" << endl;
            
        } catch (const exception& e) {
            auto end = chrono::high_resolution_clock::now();
            auto duration_us = chrono::duration_cast<chrono::microseconds>(end - start);
            auto duration_s = chrono::duration_cast<chrono::duration<double>>(end - start);
            
            result.time_us = duration_us.count();
            result.time_seconds = duration_s.count();
            result.error_msg = e.what();
            result.clique_size = 0;
            result.valid = false;
            
            cout << "  ✗ Error: " << e.what() << endl;
        }
        
        results.push_back(result);
        print_result(result);
    }
    
    // Summary
    cout << "\n";
    cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    cout << "║                         BENCHMARK SUMMARY                          ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    cout << endl;
    
    cout << left << setw(25) << "Algorithm" 
              << right << setw(15) << "Time (s)" 
              << setw(12) << "Clique" 
              << setw(10) << "Valid" << endl;
    cout << string(62, '-') << endl;
    
    for (const auto& r : results) {
        cout << left << setw(25) << r.algorithm_name;
        
        if (!r.error_msg.empty()) {
            cout << right << setw(15) << "ERROR" 
                      << setw(12) << "-" 
                      << setw(10) << "-" << endl;
        } else {
            cout << right << setw(15) << fixed << setprecision(4) << r.time_seconds
                      << setw(12) << r.clique_size
                      << setw(10) << (r.valid ? "YES" : "NO") << endl;
        }
    }
    
    cout << string(62, '-') << endl;
    
    // Find best clique
    int max_clique_size = 0;
    string best_algorithm = "";
    for (const auto& r : results) {
        if (r.valid && r.clique_size > max_clique_size) {
            max_clique_size = r.clique_size;
            best_algorithm = r.algorithm_name;
        }
    }
    
    cout << "\nBest Result: " << best_algorithm << " with clique size " << max_clique_size << endl;
    
    // Save to CSV
    string output_csv = "results/sat_benchmark_optimized.csv";
    ofstream csv(output_csv);
    if (csv.is_open()) {
        csv << "algorithm,time_us,time_seconds,clique_size,valid,error\n";
        for (const auto& r : results) {
            csv << r.algorithm_name << ","
                << r.time_us << ","
                << fixed << setprecision(4) << r.time_seconds << ","
                << r.clique_size << ","
                << (r.valid ? "true" : "false") << ","
                << r.error_msg << "\n";
        }
        csv.close();
        cout << "\nResults saved to: " << output_csv << endl;
    }
    
    return 0;
}
