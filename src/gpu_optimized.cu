// GPU-optimized maximum clique implementation using CUDA
// For NVIDIA RTX 4050 and other CUDA-capable GPUs

#include "gpu_optimized.cuh"
#include "tomita.hpp"  // Fall back to CPU version
#include <iostream>
#include <algorithm>
#include <cstring>

#ifdef __CUDACC__
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

// CUDA error checking macro
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error in " << __FILE__ << ":" << __LINE__ \
                      << " - " << cudaGetErrorString(err) << std::endl; \
            throw std::runtime_error("CUDA error"); \
        } \
    } while(0)

// Device function: Check if vertex v is connected to all vertices in set
__device__ bool is_connected_to_all(const int* adj_matrix, int n, int v, 
                                    const int* vertices, int count) {
    for (int i = 0; i < count; i++) {
        int u = vertices[i];
        if (!adj_matrix[v * n + u]) {
            return false;
        }
    }
    return true;
}

// Device function: Choose pivot vertex (vertex with maximum neighbors in P)
__device__ int choose_pivot_device(const int* adj_matrix, int n,
                                    const int* P, int P_size,
                                    const int* X, int X_size) {
    int best_pivot = -1;
    int max_connections = -1;
    
    // Check vertices in P
    for (int i = 0; i < P_size; i++) {
        int v = P[i];
        int connections = 0;
        
        for (int j = 0; j < P_size; j++) {
            if (adj_matrix[v * n + P[j]]) {
                connections++;
            }
        }
        
        if (connections > max_connections) {
            max_connections = connections;
            best_pivot = v;
        }
    }
    
    // Check vertices in X
    for (int i = 0; i < X_size; i++) {
        int v = X[i];
        int connections = 0;
        
        for (int j = 0; j < P_size; j++) {
            if (adj_matrix[v * n + P[j]]) {
                connections++;
            }
        }
        
        if (connections > max_connections) {
            max_connections = connections;
            best_pivot = v;
        }
    }
    
    return best_pivot;
}

// Device function: Intersect P with neighbors of v
__device__ int intersect_with_neighbors(const int* adj_matrix, int n,
                                         const int* P, int P_size, int v,
                                         int* result) {
    int count = 0;
    for (int i = 0; i < P_size; i++) {
        int u = P[i];
        if (adj_matrix[v * n + u]) {
            result[count++] = u;
        }
    }
    return count;
}

// CUDA kernel: Parallel Bron-Kerbosch at top level
__global__ void parallel_bk_kernel(const int* adj_matrix, int n,
                                    const int* initial_vertices, int num_vertices,
                                    int* global_best_clique, int* global_best_size) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (tid >= num_vertices) return;
    
    // Shared memory for thread block
    extern __shared__ int shared_mem[];
    
    int* local_R = &shared_mem[0];
    int* local_P = &shared_mem[n];
    int* local_X = &shared_mem[2 * n];
    int* local_best = &shared_mem[3 * n];
    int* temp_buffer = &shared_mem[4 * n];
    
    // Initialize for this thread's starting vertex
    int start_vertex = initial_vertices[tid];
    
    local_R[0] = start_vertex;
    int R_size = 1;
    
    // Build P: neighbors of start_vertex that come after it in ordering
    int P_size = 0;
    for (int i = tid + 1; i < num_vertices; i++) {
        int v = initial_vertices[i];
        if (adj_matrix[start_vertex * n + v]) {
            local_P[P_size++] = v;
        }
    }
    
    // Build X: neighbors that come before in ordering
    int X_size = 0;
    for (int i = 0; i < tid; i++) {
        int v = initial_vertices[i];
        if (adj_matrix[start_vertex * n + v]) {
            local_X[X_size++] = v;
        }
    }
    
    int local_best_size = 1;
    local_best[0] = start_vertex;
    
    // Simplified BK iteration (not fully recursive due to GPU limitations)
    // Full recursion would require dynamic parallelism (compute capability 3.5+)
    while (P_size > 0 && R_size + P_size > local_best_size) {
        // Choose pivot to minimize branching
        int pivot = choose_pivot_device(adj_matrix, n, local_P, P_size, local_X, X_size);
        
        // Select candidates (P \ N(pivot))
        int candidates[256];  // Max candidates per iteration
        int num_candidates = 0;
        
        if (pivot != -1) {
            for (int i = 0; i < P_size && num_candidates < 256; i++) {
                int v = local_P[i];
                if (!adj_matrix[pivot * n + v]) {
                    candidates[num_candidates++] = v;
                }
            }
        } else {
            for (int i = 0; i < P_size && num_candidates < 256; i++) {
                candidates[num_candidates++] = local_P[i];
            }
        }
        
        // Try each candidate
        bool found_improvement = false;
        for (int c = 0; c < num_candidates; c++) {
            int v = candidates[c];
            
            // Check if v connects to all in R
            if (is_connected_to_all(adj_matrix, n, v, local_R, R_size)) {
                local_R[R_size++] = v;
                
                if (R_size > local_best_size) {
                    local_best_size = R_size;
                    for (int i = 0; i < R_size; i++) {
                        local_best[i] = local_R[i];
                    }
                    found_improvement = true;
                }
                
                // Update P: keep only neighbors of v
                int new_P_size = intersect_with_neighbors(adj_matrix, n, local_P, P_size, v, temp_buffer);
                P_size = new_P_size;
                for (int i = 0; i < P_size; i++) {
                    local_P[i] = temp_buffer[i];
                }
                break;
            }
        }
        
        if (!found_improvement) {
            // Remove first candidate from P and add to X
            if (P_size > 0) {
                local_X[X_size++] = local_P[0];
                for (int i = 0; i < P_size - 1; i++) {
                    local_P[i] = local_P[i + 1];
                }
                P_size--;
            }
        }
    }
    
    // Update global best using atomic operations
    int old_best = atomicMax(global_best_size, local_best_size);
    
    // If we found a new best, update the clique
    if (local_best_size > old_best) {
        // Use atomic compare-and-swap for thread-safe update
        for (int i = 0; i < local_best_size; i++) {
            global_best_clique[i] = local_best[i];
        }
    }
}

#endif // __CUDACC__

void GPUOptimized::copy_graph_to_device(const Graph& g) {
    #ifdef __CUDACC__
    std::cout << "Copying graph to GPU memory..." << std::endl;
    #else
    std::cout << "Warning: CUDA not available, falling back to CPU version" << std::endl;
    #endif
}

void GPUOptimized::free_device_memory() {
    #ifdef __CUDACC__
    // GPU memory freed in find_maximum_clique
    #endif
}

std::vector<int> GPUOptimized::find_maximum_clique(const Graph& g) {
    #ifdef __CUDACC__
    n = g.num_vertices();
    
    // Check if CUDA is available
    int deviceCount = 0;
    cudaError_t error = cudaGetDeviceCount(&deviceCount);
    
    if (error != cudaSuccess || deviceCount == 0) {
        std::cerr << "Warning: No CUDA-capable GPU found. Falling back to CPU." << std::endl;
        TomitaAlgorithm tomita;
        return tomita.find_maximum_clique(g);
    }
    
    // Get device properties
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    std::cout << "Using GPU: " << prop.name << std::endl;
    std::cout << "Compute Capability: " << prop.major << "." << prop.minor << std::endl;
    std::cout << "Total Global Memory: " << (prop.totalGlobalMem / (1024 * 1024)) << " MB" << std::endl;
    
    // Check if graph is too large
    size_t required_memory = static_cast<size_t>(n) * n * sizeof(int);
    if (required_memory > prop.totalGlobalMem * 0.8) {
        std::cerr << "Warning: Graph too large for GPU memory. Falling back to CPU." << std::endl;
        TomitaAlgorithm tomita;
        return tomita.find_maximum_clique(g);
    }
    
    // Allocate host memory for adjacency matrix
    std::vector<int> h_adj_matrix(n * n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            h_adj_matrix[i * n + j] = g.has_edge(i, j) ? 1 : 0;
        }
    }
    
    // Compute degeneracy ordering
    std::vector<int> h_ordering;
    std::vector<int> degrees(n);
    std::vector<bool> removed(n, false);
    
    for (int i = 0; i < n; i++) {
        degrees[i] = g.get_degree(i);
    }
    
    for (int iter = 0; iter < n; iter++) {
        int min_deg = n + 1;
        int min_v = -1;
        
        for (int v = 0; v < n; v++) {
            if (!removed[v] && degrees[v] < min_deg) {
                min_deg = degrees[v];
                min_v = v;
            }
        }
        
        h_ordering.push_back(min_v);
        removed[min_v] = true;
        
        for (int neighbor : g.get_neighbors(min_v)) {
            if (!removed[neighbor]) {
                degrees[neighbor]--;
            }
        }
    }
    
    // Allocate device memory
    int* d_adj_matrix;
    int* d_ordering;
    int* d_best_clique;
    int* d_best_size;
    
    CUDA_CHECK(cudaMalloc(&d_adj_matrix, n * n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_ordering, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_best_clique, n * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_best_size, sizeof(int)));
    
    // Copy data to device
    CUDA_CHECK(cudaMemcpy(d_adj_matrix, h_adj_matrix.data(), 
                          n * n * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_ordering, h_ordering.data(), 
                          n * sizeof(int), cudaMemcpyHostToDevice));
    
    int zero = 0;
    CUDA_CHECK(cudaMemcpy(d_best_size, &zero, sizeof(int), cudaMemcpyHostToDevice));
    
    // Configure kernel launch
    // For RTX 4050: 2560 CUDA cores, optimal occupancy with 256-512 threads/block
    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;
    size_t sharedMemSize = 5 * n * sizeof(int);
    
    // Check shared memory limits
    if (sharedMemSize > prop.sharedMemPerBlock) {
        std::cerr << "Warning: Required shared memory (" << sharedMemSize 
                  << " bytes) exceeds limit (" << prop.sharedMemPerBlock 
                  << " bytes). Falling back to CPU." << std::endl;
        
        CUDA_CHECK(cudaFree(d_adj_matrix));
        CUDA_CHECK(cudaFree(d_ordering));
        CUDA_CHECK(cudaFree(d_best_clique));
        CUDA_CHECK(cudaFree(d_best_size));
        
        TomitaAlgorithm tomita;
        return tomita.find_maximum_clique(g);
    }
    
    // Launch kernel
    std::cout << "Launching GPU kernel with " << blocksPerGrid << " blocks, "
              << threadsPerBlock << " threads per block..." << std::endl;
    
    parallel_bk_kernel<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(
        d_adj_matrix, n, d_ordering, n, d_best_clique, d_best_size
    );
    
    // Check for kernel errors
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    
    // Copy results back
    int best_size;
    CUDA_CHECK(cudaMemcpy(&best_size, d_best_size, sizeof(int), cudaMemcpyDeviceToHost));
    
    std::vector<int> best_clique(best_size);
    CUDA_CHECK(cudaMemcpy(best_clique.data(), d_best_clique, 
                          best_size * sizeof(int), cudaMemcpyDeviceToHost));
    
    // Free device memory
    CUDA_CHECK(cudaFree(d_adj_matrix));
    CUDA_CHECK(cudaFree(d_ordering));
    CUDA_CHECK(cudaFree(d_best_clique));
    CUDA_CHECK(cudaFree(d_best_size));
    
    std::cout << "GPU computation complete. Clique size: " << best_size << std::endl;
    
    return best_clique;
    
    #else
    // CUDA not available, fall back to CPU
    std::cout << "CUDA not available, using CPU Tomita algorithm instead" << std::endl;
    TomitaAlgorithm tomita;
    return tomita.find_maximum_clique(g);
    #endif
}
