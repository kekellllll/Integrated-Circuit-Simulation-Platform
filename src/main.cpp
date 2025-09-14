#include "core/circuit.h"
#include "core/cuda_engine.h"
#include "plugins/plugin_system.h"
#include <iostream>
#include <memory>

using namespace ic_sim;

int main() {
    std::cout << "=== Integrated Circuit Simulation Platform ===" << std::endl;
    std::cout << "Initializing simulation environment..." << std::endl;
    
    // Initialize CUDA engine
    CudaSimulationEngine cuda_engine;
    if (cuda_engine.initialize()) {
        std::cout << "CUDA acceleration: Available" << std::endl;
        std::cout << "CUDA devices: " << CudaSimulationEngine::getDeviceCount() << std::endl;
        std::cout << "Device info: " << CudaSimulationEngine::getDeviceInfo() << std::endl;
    } else {
        std::cout << "CUDA acceleration: Not available, using CPU fallback" << std::endl;
    }
    
    // Initialize plugin system
    PluginManager& plugin_manager = PluginManager::getInstance();
    std::cout << "Plugin system initialized" << std::endl;
    
    // Discover and load plugins
    auto discovered_plugins = plugin_manager.discoverPlugins("plugins/");
    std::cout << "Discovered " << discovered_plugins.size() << " plugin(s)" << std::endl;
    
    for (const auto& plugin_path : discovered_plugins) {
        if (plugin_manager.loadPlugin(plugin_path)) {
            std::cout << "Loaded plugin: " << plugin_path << std::endl;
        }
    }
    
    // Create a demo circuit
    std::cout << "\\nCreating demo circuit..." << std::endl;
    auto circuit = std::make_shared<Circuit>("Demo RC Circuit");
    
    // Create nodes
    auto node1 = std::make_shared<Node>("N1");
    auto node2 = std::make_shared<Node>("N2");
    auto node3 = std::make_shared<Node>("GND");
    
    circuit->addNode(node1);
    circuit->addNode(node2);
    circuit->addNode(node3);
    
    // Create components
    auto resistor = std::make_shared<Resistor>(1000.0); // 1k ohm
    resistor->setId("R1");
    resistor->connect(node1);
    resistor->connect(node2);
    
    auto capacitor = std::make_shared<Capacitor>(1e-6); // 1uF
    capacitor->setId("C1");
    capacitor->connect(node2);
    capacitor->connect(node3);
    
    circuit->addComponent(resistor);
    circuit->addComponent(capacitor);
    
    // Set initial conditions
    node1->setVoltage(5.0); // 5V input
    node3->setVoltage(0.0); // Ground
    
    std::cout << "Circuit created with " << 2 << " components and " << 3 << " nodes" << std::endl;
    
    // Run simulation
    std::cout << "\\nStarting simulation..." << std::endl;
    double duration = 0.01; // 10ms
    double timestep = 1e-6; // 1us
    
    circuit->simulate(duration, timestep);
    
    // Display results
    std::cout << "\\nSimulation Results:" << std::endl;
    std::cout << "R1 current: " << resistor->getCurrentValue() << " A" << std::endl;
    std::cout << "C1 voltage: " << capacitor->getCurrentValue() << " V" << std::endl;
    
    // Test CUDA acceleration with matrix operations
    if (cuda_engine.isAvailable()) {
        std::cout << "\\nTesting CUDA acceleration..." << std::endl;
        
        // Create test matrices for circuit analysis
        std::vector<std::vector<double>> matrix = {
            {1000.0, -1000.0, 0.0},
            {-1000.0, 1000.0 + 1e6, -1e6},
            {0.0, -1e6, 1e6}
        };
        
        std::vector<double> rhs = {5.0, 0.0, 0.0};
        std::vector<double> solution;
        
        if (cuda_engine.solveLinearSystem(matrix, rhs, solution)) {
            std::cout << "CUDA linear solver results:" << std::endl;
            for (size_t i = 0; i < solution.size(); ++i) {
                std::cout << "  x[" << i << "] = " << solution[i] << std::endl;
            }
        }
    }
    
    // Plugin system demonstration
    auto loaded_plugins = plugin_manager.getLoadedPlugins();
    if (!loaded_plugins.empty()) {
        std::cout << "\\nTesting plugin system..." << std::endl;
        std::cout << "Loaded plugins: ";
        for (const auto& name : loaded_plugins) {
            std::cout << name << " ";
        }
        std::cout << std::endl;
        
        auto supported_components = plugin_manager.getAllSupportedComponents();
        std::cout << "Supported component types: ";
        for (const auto& type : supported_components) {
            std::cout << type << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\\nSimulation completed successfully!" << std::endl;
    std::cout << "Web interface available at: web/index.html" << std::endl;
    
    return 0;
}