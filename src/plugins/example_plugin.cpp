#include "plugins/plugin_system.h"
#include "core/circuit.h"
#include <memory>
#include <iostream>
#include <cmath>

namespace ic_sim {

/**
 * Example Inductor component for plugin demonstration
 */
class Inductor : public Component {
public:
    Inductor(double inductance) : inductance_(inductance), current_(0.0), voltage_(0.0) {}
    
    void simulate(double timestep) override {
        if (nodes_.size() >= 2) {
            double voltage_diff = nodes_[0]->getVoltage() - nodes_[1]->getVoltage();
            // L * di/dt = V, so di = V * dt / L
            double current_change = voltage_diff * timestep / inductance_;
            current_ += current_change;
            voltage_ = voltage_diff;
        }
    }
    
    double getCurrentValue() const override { return current_; }
    void connect(std::shared_ptr<Node> node) override {
        nodes_.push_back(node);
        node->addComponent(std::static_pointer_cast<Component>(shared_from_this()));
    }
    std::string getType() const override { return "Inductor"; }
    
    double getInductance() const { return inductance_; }

private:
    double inductance_;
    double current_;
    double voltage_;
};

/**
 * Example Diode component
 */
class Diode : public Component {
public:
    Diode(double forward_voltage = 0.7) : forward_voltage_(forward_voltage), current_(0.0) {}
    
    void simulate(double timestep) override {
        if (nodes_.size() >= 2) {
            double voltage_diff = nodes_[0]->getVoltage() - nodes_[1]->getVoltage();
            
            if (voltage_diff > forward_voltage_) {
                // Forward biased - exponential current
                current_ = 1e-12 * (std::exp(voltage_diff / 0.026) - 1);
            } else {
                // Reverse biased - minimal leakage current
                current_ = -1e-12;
            }
        }
    }
    
    double getCurrentValue() const override { return current_; }
    void connect(std::shared_ptr<Node> node) override {
        nodes_.push_back(node);
        node->addComponent(std::static_pointer_cast<Component>(shared_from_this()));
    }
    std::string getType() const override { return "Diode"; }

private:
    double forward_voltage_;
    double current_;
};

/**
 * Example plugin implementation
 */
class ExamplePlugin : public BasePlugin {
public:
    ExamplePlugin() : BasePlugin("ExamplePlugin", "1.0.0", "Example plugin with inductor and diode components") {}
    
    std::shared_ptr<Component> createComponent(const std::string& type,
                                              const std::map<std::string, double>& parameters) override {
        if (type == "Inductor") {
            double inductance = 1e-3; // Default 1mH
            auto it = parameters.find("inductance");
            if (it != parameters.end()) {
                inductance = it->second;
            }
            return std::make_shared<Inductor>(inductance);
        }
        
        if (type == "Diode") {
            double forward_voltage = 0.7; // Default 0.7V
            auto it = parameters.find("forward_voltage");
            if (it != parameters.end()) {
                forward_voltage = it->second;
            }
            return std::make_shared<Diode>(forward_voltage);
        }
        
        return nullptr;
    }
    
    std::vector<std::string> getSupportedComponents() const override {
        return {"Inductor", "Diode"};
    }

protected:
    bool doInitialize() override {
        std::cout << "ExamplePlugin initialized with components: Inductor, Diode" << std::endl;
        return true;
    }
    
    void doCleanup() override {
        std::cout << "ExamplePlugin cleaned up" << std::endl;
    }
};

} // namespace ic_sim

// Export plugin
DECLARE_PLUGIN(ic_sim::ExamplePlugin)