#include "core/cuda_engine.h"
#include <iostream>
#include <cstring>

// CPU-only implementation when CUDA is not available

namespace ic_sim {

CudaSimulationEngine::CudaSimulationEngine() 
    : cuda_initialized_(false), d_matrix_(nullptr), d_vector_(nullptr), 
      d_solution_(nullptr), allocated_size_(0) {
}

CudaSimulationEngine::~CudaSimulationEngine() {
    cleanup();
}

bool CudaSimulationEngine::initialize() {
    std::cout << "CUDA not available. Using CPU fallback simulation." << std::endl;
    return false;
}

void CudaSimulationEngine::cleanup() {
    // Nothing to cleanup in CPU version
}

bool CudaSimulationEngine::solveLinearSystem(const std::vector<std::vector<double>>& matrix,
                                            const std::vector<double>& rhs,
                                            std::vector<double>& solution) {
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
}

bool CudaSimulationEngine::simulateComponents(std::vector<double>& voltages,
                                             std::vector<double>& currents,
                                             const std::vector<double>& resistances,
                                             double timestep,
                                             int num_components) {
    // CPU fallback
    currents.resize(num_components);
    for (int i = 0; i < num_components; i++) {
        if (resistances[i] > 0.0) {
            currents[i] = voltages[i] / resistances[i];
        }
    }
    return true;
}

bool CudaSimulationEngine::isAvailable() {
    return false;
}

int CudaSimulationEngine::getDeviceCount() {
    return 0;
}

std::string CudaSimulationEngine::getDeviceInfo(int device_id) {
    return "No CUDA device available";
}

bool CudaSimulationEngine::allocateDeviceMemory(size_t size) {
    return false;
}

void CudaSimulationEngine::freeDeviceMemory() {
    // Nothing to free
}

} // namespace ic_sim