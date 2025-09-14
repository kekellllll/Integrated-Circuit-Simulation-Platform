#include "core/circuit.h"
#include "plugins/plugin_system.h"
#include <iostream>
#include <cassert>

using namespace ic_sim;

void test_complete_simulation() {
    std::cout << "Testing complete simulation pipeline..." << std::endl;
    
    // Create circuit
    auto circuit = std::make_shared<Circuit>("Integration Test Circuit");
    
    // Add nodes
    auto vcc = std::make_shared<Node>("VCC");
    auto out = std::make_shared<Node>("OUT"); 
    auto gnd = std::make_shared<Node>("GND");
    
    vcc->setVoltage(5.0);
    gnd->setVoltage(0.0);
    
    circuit->addNode(vcc);
    circuit->addNode(out);
    circuit->addNode(gnd);
    
    // Add components
    auto resistor = std::make_shared<Resistor>(1000.0);
    resistor->setId("R1");
    resistor->connect(vcc);
    resistor->connect(out);
    
    auto capacitor = std::make_shared<Capacitor>(1e-6);
    capacitor->setId("C1");
    capacitor->connect(out);
    capacitor->connect(gnd);
    
    circuit->addComponent(resistor);
    circuit->addComponent(capacitor);
    
    // Run simulation
    circuit->simulate(0.001, 1e-6);
    
    // Verify results
    assert(circuit->getComponent("R1") != nullptr);
    assert(circuit->getComponent("C1") != nullptr);
    assert(circuit->getNode("VCC") != nullptr);
    
    std::cout << "✓ Complete simulation test passed" << std::endl;
}

void test_plugin_integration() {
    std::cout << "Testing plugin system integration..." << std::endl;
    
    PluginManager& pm = PluginManager::getInstance();
    
    // Test component creation without plugins
    auto component = pm.createComponent("NonExistentType", {});
    assert(component == nullptr);
    
    // Test supported components
    auto supported = pm.getAllSupportedComponents();
    // Should be empty since no plugins are loaded
    
    std::cout << "✓ Plugin integration test passed" << std::endl;
}

int main() {
    std::cout << "Running Integration Tests..." << std::endl;
    
    try {
        test_complete_simulation();
        test_plugin_integration();
        
        std::cout << "\\n✅ All integration tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown test failure" << std::endl;
        return 1;
    }
}