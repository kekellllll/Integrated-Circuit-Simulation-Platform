#include "core/circuit.h"
#include <iostream>
#include <memory>
#include <iomanip>
#include <cmath>

using namespace ic_sim;

int main() {
    std::cout << "RC Circuit Example" << std::endl;
    std::cout << "==================" << std::endl;
    
    // Create circuit
    auto circuit = std::make_shared<Circuit>("RC Low-pass Filter");
    
    // Create nodes
    auto vin = std::make_shared<Node>("VIN");
    auto vout = std::make_shared<Node>("VOUT");
    auto gnd = std::make_shared<Node>("GND");
    
    circuit->addNode(vin);
    circuit->addNode(vout);
    circuit->addNode(gnd);
    
    // Create components
    auto resistor = std::make_shared<Resistor>(1000.0); // 1kΩ
    resistor->setId("R1");
    resistor->connect(vin);
    resistor->connect(vout);
    
    auto capacitor = std::make_shared<Capacitor>(1e-6); // 1μF
    capacitor->setId("C1");
    capacitor->connect(vout);
    capacitor->connect(gnd);
    
    circuit->addComponent(resistor);
    circuit->addComponent(capacitor);
    
    // Set input voltage (step response)
    vin->setVoltage(5.0); // 5V step input
    gnd->setVoltage(0.0); // Ground reference
    
    std::cout << "Circuit created:" << std::endl;
    std::cout << "- R1: " << resistor->getResistance() << " Ω" << std::endl;
    std::cout << "- C1: " << capacitor->getCapacitance() * 1e6 << " μF" << std::endl;
    std::cout << "- Cutoff frequency: " << 1.0 / (2 * M_PI * resistor->getResistance() * capacitor->getCapacitance()) << " Hz" << std::endl;
    
    // Run simulation
    std::cout << "\\nRunning transient simulation..." << std::endl;
    double duration = 0.01; // 10ms
    double timestep = 1e-5; // 10μs
    int steps = static_cast<int>(duration / timestep);
    
    std::cout << "Time(ms)\\tVout(V)\\tCurrent(mA)" << std::endl;
    std::cout << "--------\\t-------\\t-----------" << std::endl;
    
    for (int i = 0; i <= steps; i += steps/20) {
        double time = i * timestep;
        
        // Simulate one timestep
        resistor->simulate(timestep);
        capacitor->simulate(timestep);
        
        // Display results every 5% of simulation
        if (i % (steps/20) == 0) {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << time * 1000 << "\\t\\t";
            std::cout << vout->getVoltage() << "\\t\\t";
            std::cout << resistor->getCurrentValue() * 1000 << std::endl;
        }
    }
    
    std::cout << "\\nSimulation completed!" << std::endl;
    std::cout << "Final output voltage: " << vout->getVoltage() << " V" << std::endl;
    
    return 0;
}