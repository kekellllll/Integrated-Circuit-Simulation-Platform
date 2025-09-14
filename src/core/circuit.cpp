#include "core/circuit.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace ic_sim {

void Circuit::addComponent(std::shared_ptr<Component> component) {
    if (component && !component->getId().empty()) {
        components_[component->getId()] = component;
    }
}

void Circuit::addNode(std::shared_ptr<Node> node) {
    if (node && !node->getId().empty()) {
        nodes_[node->getId()] = node;
    }
}

void Circuit::simulate(double duration, double timestep) {
    std::cout << "Simulating circuit '" << name_ << "' for " << duration 
              << "s with timestep " << timestep << "s" << std::endl;
    
    double time = 0.0;
    while (time < duration) {
        // Simulate all components
        for (auto& [id, component] : components_) {
            component->simulate(timestep);
        }
        time += timestep;
    }
    
    std::cout << "Simulation completed." << std::endl;
}

void Circuit::reset() {
    // Reset all nodes to zero voltage
    for (auto& [id, node] : nodes_) {
        node->setVoltage(0.0);
    }
    
    std::cout << "Circuit '" << name_ << "' reset." << std::endl;
}

std::shared_ptr<Node> Circuit::getNode(const std::string& id) {
    auto it = nodes_.find(id);
    return (it != nodes_.end()) ? it->second : nullptr;
}

std::shared_ptr<Component> Circuit::getComponent(const std::string& id) {
    auto it = components_.find(id);
    return (it != components_.end()) ? it->second : nullptr;
}

// Resistor implementation
void Resistor::simulate(double timestep) {
    if (nodes_.size() >= 2) {
        double voltage_diff = nodes_[0]->getVoltage() - nodes_[1]->getVoltage();
        current_ = voltage_diff / resistance_; // Ohm's law: I = V/R
    }
}

void Resistor::connect(std::shared_ptr<Node> node) {
    nodes_.push_back(node);
    node->addComponent(std::static_pointer_cast<Component>(shared_from_this()));
}

// Capacitor implementation
void Capacitor::simulate(double timestep) {
    if (nodes_.size() >= 2) {
        double new_voltage = nodes_[0]->getVoltage() - nodes_[1]->getVoltage();
        double current = capacitance_ * (new_voltage - voltage_) / timestep; // I = C * dV/dt
        charge_ += current * timestep;
        voltage_ = new_voltage;
    }
}

void Capacitor::connect(std::shared_ptr<Node> node) {
    nodes_.push_back(node);
    node->addComponent(std::static_pointer_cast<Component>(shared_from_this()));
}

} // namespace ic_sim