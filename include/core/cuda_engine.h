#pragma once

#include <vector>
#include <memory>

namespace ic_sim {

/**
 * CUDA-accelerated simulation engine
 * Provides GPU acceleration for large circuit simulations
 */
class CudaSimulationEngine {
public:
    CudaSimulationEngine();
    ~CudaSimulationEngine();
    
    // Initialize CUDA context and memory
    bool initialize();
    void cleanup();
    
    // Matrix operations for circuit analysis
    bool solveLinearSystem(const std::vector<std::vector<double>>& matrix,
                          const std::vector<double>& rhs,
                          std::vector<double>& solution);
    
    // Parallel component simulation
    bool simulateComponents(std::vector<double>& voltages,
                           std::vector<double>& currents,
                           const std::vector<double>& resistances,
                           double timestep,
                           int num_components);
    
    // Get device information
    static bool isAvailable();
    static int getDeviceCount();
    static std::string getDeviceInfo(int device_id = 0);

private:
    bool cuda_initialized_;
    void* d_matrix_;  // Device memory pointers
    void* d_vector_;
    void* d_solution_;
    size_t allocated_size_;
    
    bool allocateDeviceMemory(size_t size);
    void freeDeviceMemory();
};

} // namespace ic_sim