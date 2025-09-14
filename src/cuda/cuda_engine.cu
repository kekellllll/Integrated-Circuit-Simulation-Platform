#include "core/cuda_engine.h"
#include <iostream>
#include <cstring>

#ifdef __CUDACC__
#include <cuda_runtime.h>
#include <cublas_v2.h>

__global__ void simulateResistorsKernel(double* voltages, double* currents, 
                                       const double* resistances, int num_components) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < num_components) {
        // Simple Ohm's law calculation: I = V/R
        if (resistances[idx] > 0.0) {
            currents[idx] = voltages[idx] / resistances[idx];
        }
    }
}

__global__ void matrixVectorMultiply(const double* matrix, const double* vector,
                                    double* result, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += matrix[idx * n + j] * vector[j];
        }
        result[idx] = sum;
    }
}

#endif

namespace ic_sim {

CudaSimulationEngine::CudaSimulationEngine() 
    : cuda_initialized_(false), d_matrix_(nullptr), d_vector_(nullptr), 
      d_solution_(nullptr), allocated_size_(0) {
}

CudaSimulationEngine::~CudaSimulationEngine() {
    cleanup();
}

bool CudaSimulationEngine::initialize() {
#ifdef __CUDACC__
    if (cuda_initialized_) return true;
    
    int device_count = 0;
    cudaError_t err = cudaGetDeviceCount(&device_count);
    if (err != cudaSuccess || device_count == 0) {
        std::cerr << "No CUDA devices available: " << cudaGetErrorString(err) << std::endl;
        return false;
    }
    
    err = cudaSetDevice(0);
    if (err != cudaSuccess) {
        std::cerr << "Failed to set CUDA device: " << cudaGetErrorString(err) << std::endl;
        return false;
    }
    
    cuda_initialized_ = true;
    std::cout << "CUDA simulation engine initialized successfully" << std::endl;
    return true;
#else
    std::cout << "CUDA support not compiled. Using CPU fallback." << std::endl;
    return false;
#endif
}

void CudaSimulationEngine::cleanup() {
#ifdef __CUDACC__
    if (cuda_initialized_) {
        freeDeviceMemory();
        cudaDeviceReset();
        cuda_initialized_ = false;
    }
#endif
}

bool CudaSimulationEngine::solveLinearSystem(const std::vector<std::vector<double>>& matrix,
                                            const std::vector<double>& rhs,
                                            std::vector<double>& solution) {
#ifdef __CUDACC__
    if (!cuda_initialized_) return false;
    
    int n = matrix.size();
    solution.resize(n);
    
    // For now, implement a simple iterative solver on GPU
    // In a real implementation, you'd use cuSolver for LU decomposition
    
    // Copy data to GPU and solve (simplified implementation)
    size_t matrix_size = n * n * sizeof(double);
    size_t vector_size = n * sizeof(double);
    
    if (!allocateDeviceMemory(matrix_size + 2 * vector_size)) {
        return false;
    }
    
    // Flatten matrix for GPU
    std::vector<double> flat_matrix(n * n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            flat_matrix[i * n + j] = matrix[i][j];
        }
    }
    
    cudaMemcpy(d_matrix_, flat_matrix.data(), matrix_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vector_, rhs.data(), vector_size, cudaMemcpyHostToDevice);
    
    // Simple matrix-vector multiply as placeholder
    dim3 blockSize(256);
    dim3 gridSize((n + blockSize.x - 1) / blockSize.x);
    
    matrixVectorMultiply<<<gridSize, blockSize>>>((double*)d_matrix_, (double*)d_vector_, 
                                                  (double*)d_solution_, n);
    
    cudaMemcpy(solution.data(), d_solution_, vector_size, cudaMemcpyDeviceToHost);
    
    return cudaGetLastError() == cudaSuccess;
#else
    // CPU fallback implementation
    int n = matrix.size();
    solution.resize(n, 0.0);
    
    // Simple Gauss-Seidel iteration
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < n; i++) {
            double sum = rhs[i];
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    sum -= matrix[i][j] * solution[j];
                }
            }
            if (matrix[i][i] != 0.0) {
                solution[i] = sum / matrix[i][i];
            }
        }
    }
    return true;
#endif
}

bool CudaSimulationEngine::simulateComponents(std::vector<double>& voltages,
                                             std::vector<double>& currents,
                                             const std::vector<double>& resistances,
                                             double timestep,
                                             int num_components) {
#ifdef __CUDACC__
    if (!cuda_initialized_) return false;
    
    size_t size = num_components * sizeof(double);
    
    if (!allocateDeviceMemory(3 * size)) {
        return false;
    }
    
    // Copy data to GPU
    cudaMemcpy(d_matrix_, voltages.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_vector_, resistances.data(), size, cudaMemcpyHostToDevice);
    
    // Launch kernel
    dim3 blockSize(256);
    dim3 gridSize((num_components + blockSize.x - 1) / blockSize.x);
    
    simulateResistorsKernel<<<gridSize, blockSize>>>((double*)d_matrix_, (double*)d_solution_,
                                                     (double*)d_vector_, num_components);
    
    // Copy results back
    cudaMemcpy(currents.data(), d_solution_, size, cudaMemcpyDeviceToHost);
    
    return cudaGetLastError() == cudaSuccess;
#else
    // CPU fallback
    currents.resize(num_components);
    for (int i = 0; i < num_components; i++) {
        if (resistances[i] > 0.0) {
            currents[i] = voltages[i] / resistances[i];
        }
    }
    return true;
#endif
}

bool CudaSimulationEngine::isAvailable() {
#ifdef __CUDACC__
    int device_count = 0;
    return (cudaGetDeviceCount(&device_count) == cudaSuccess && device_count > 0);
#else
    return false;
#endif
}

int CudaSimulationEngine::getDeviceCount() {
#ifdef __CUDACC__
    int device_count = 0;
    cudaGetDeviceCount(&device_count);
    return device_count;
#else
    return 0;
#endif
}

std::string CudaSimulationEngine::getDeviceInfo(int device_id) {
#ifdef __CUDACC__
    cudaDeviceProp prop;
    if (cudaGetDeviceProperties(&prop, device_id) == cudaSuccess) {
        return std::string("Device: ") + prop.name + 
               " (Compute " + std::to_string(prop.major) + "." + std::to_string(prop.minor) + ")";
    }
#endif
    return "No CUDA device available";
}

bool CudaSimulationEngine::allocateDeviceMemory(size_t size) {
#ifdef __CUDACC__
    if (allocated_size_ < size) {
        freeDeviceMemory();
        
        cudaError_t err = cudaMalloc(&d_matrix_, size);
        if (err != cudaSuccess) return false;
        
        err = cudaMalloc(&d_vector_, size);
        if (err != cudaSuccess) {
            cudaFree(d_matrix_);
            return false;
        }
        
        err = cudaMalloc(&d_solution_, size);
        if (err != cudaSuccess) {
            cudaFree(d_matrix_);
            cudaFree(d_vector_);
            return false;
        }
        
        allocated_size_ = size;
    }
    return true;
#else
    return false;
#endif
}

void CudaSimulationEngine::freeDeviceMemory() {
#ifdef __CUDACC__
    if (d_matrix_) { cudaFree(d_matrix_); d_matrix_ = nullptr; }
    if (d_vector_) { cudaFree(d_vector_); d_vector_ = nullptr; }
    if (d_solution_) { cudaFree(d_solution_); d_solution_ = nullptr; }
    allocated_size_ = 0;
#endif
}

} // namespace ic_sim