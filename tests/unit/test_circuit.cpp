#include "core/circuit.h"
#include <cassert>
#include <iostream>
#include <memory>

using namespace ic_sim;

void test_circuit_creation() {
    auto circuit = std::make_shared<Circuit>("Test Circuit");
    assert(circuit->getName() == "Test Circuit");
    std::cout << "✓ Circuit creation test passed" << std::endl;
}

void test_node_operations() {
    auto node = std::make_shared<Node>("N1");
    assert(node->getId() == "N1");
    assert(node->getVoltage() == 0.0);
    
    node->setVoltage(5.0);
    assert(node->getVoltage() == 5.0);
    
    std::cout << "✓ Node operations test passed" << std::endl;
}

void test_resistor_component() {
    auto resistor = std::make_shared<Resistor>(1000.0);
    assert(resistor->getResistance() == 1000.0);
    assert(resistor->getType() == "Resistor");
    
    resistor->setId("R1");
    assert(resistor->getId() == "R1");
    
    std::cout << "✓ Resistor component test passed" << std::endl;
}

void test_capacitor_component() {
    auto capacitor = std::make_shared<Capacitor>(1e-6);
    assert(capacitor->getCapacitance() == 1e-6);
    assert(capacitor->getType() == "Capacitor");
    
    capacitor->setId("C1");
    assert(capacitor->getId() == "C1");
    
    std::cout << "✓ Capacitor component test passed" << std::endl;
}

void test_circuit_simulation() {
    auto circuit = std::make_shared<Circuit>("RC Circuit");
    
    // Create nodes
    auto node1 = std::make_shared<Node>("VCC");
    auto node2 = std::make_shared<Node>("OUT");
    auto node3 = std::make_shared<Node>("GND");
    
    circuit->addNode(node1);
    circuit->addNode(node2);
    circuit->addNode(node3);
    
    // Create components
    auto resistor = std::make_shared<Resistor>(1000.0);
    resistor->setId("R1");
    
    auto capacitor = std::make_shared<Capacitor>(1e-6);
    capacitor->setId("C1");
    
    circuit->addComponent(resistor);
    circuit->addComponent(capacitor);
    
    // Verify circuit construction
    assert(circuit->getNode("VCC") != nullptr);
    assert(circuit->getNode("OUT") != nullptr);
    assert(circuit->getNode("GND") != nullptr);
    assert(circuit->getComponent("R1") != nullptr);
    assert(circuit->getComponent("C1") != nullptr);
    
    // Run simulation
    circuit->simulate(0.001, 1e-6);
    
    std::cout << "✓ Circuit simulation test passed" << std::endl;
}

void test_component_connections() {
    auto node1 = std::make_shared<Node>("N1");
    auto node2 = std::make_shared<Node>("N2");
    
    node1->setVoltage(5.0);
    node2->setVoltage(0.0);
    
    auto resistor = std::make_shared<Resistor>(1000.0);
    resistor->connect(node1);
    resistor->connect(node2);
    
    // Simulate one timestep
    resistor->simulate(1e-6);
    
    // Verify current calculation (I = V/R = 5V/1000Ω = 0.005A)
    double expected_current = 5.0 / 1000.0;
    double actual_current = resistor->getCurrentValue();
    double tolerance = 1e-9;
    
    assert(std::abs(actual_current - expected_current) < tolerance);
    
    std::cout << "✓ Component connections test passed" << std::endl;
}

int main() {
    std::cout << "Running Circuit Tests..." << std::endl;
    
    try {
        test_circuit_creation();
        test_node_operations();
        test_resistor_component();
        test_capacitor_component();
        test_circuit_simulation();
        test_component_connections();
        
        std::cout << "\\n✅ All circuit tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown test failure" << std::endl;
        return 1;
    }
}