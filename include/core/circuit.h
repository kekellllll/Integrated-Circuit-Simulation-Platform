#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace ic_sim {

// Forward declarations
class Component;
class Node;
class Circuit;

/**
 * Abstract base class for all circuit components
 * Implements the Strategy pattern for different component behaviors
 */
class Component : public std::enable_shared_from_this<Component> {
public:
    virtual ~Component() = default;
    
    virtual void simulate(double timestep) = 0;
    virtual double getCurrentValue() const = 0;
    virtual void connect(std::shared_ptr<Node> node) = 0;
    virtual std::string getType() const = 0;
    
    void setId(const std::string& id) { id_ = id; }
    std::string getId() const { return id_; }

protected:
    std::string id_;
    std::vector<std::shared_ptr<Node>> nodes_;
};

/**
 * Node class representing circuit connection points
 */
class Node {
public:
    Node(const std::string& id) : id_(id), voltage_(0.0) {}
    
    void setVoltage(double voltage) { voltage_ = voltage; }
    double getVoltage() const { return voltage_; }
    
    void addComponent(std::shared_ptr<Component> component) {
        connected_components_.push_back(component);
    }
    
    std::string getId() const { return id_; }

private:
    std::string id_;
    double voltage_;
    std::vector<std::weak_ptr<Component>> connected_components_;
};

/**
 * Circuit class implementing the Composite pattern
 * Manages collections of components and nodes
 */
class Circuit {
public:
    Circuit(const std::string& name) : name_(name) {}
    
    void addComponent(std::shared_ptr<Component> component);
    void addNode(std::shared_ptr<Node> node);
    
    void simulate(double duration, double timestep);
    void reset();
    
    std::shared_ptr<Node> getNode(const std::string& id);
    std::shared_ptr<Component> getComponent(const std::string& id);
    
    std::string getName() const { return name_; }

private:
    std::string name_;
    std::map<std::string, std::shared_ptr<Component>> components_;
    std::map<std::string, std::shared_ptr<Node>> nodes_;
};

/**
 * Resistor component implementation
 */
class Resistor : public Component {
public:
    Resistor(double resistance) : resistance_(resistance), current_(0.0) {}
    
    void simulate(double timestep) override;
    double getCurrentValue() const override { return current_; }
    void connect(std::shared_ptr<Node> node) override;
    std::string getType() const override { return "Resistor"; }
    
    double getResistance() const { return resistance_; }

private:
    double resistance_;
    double current_;
};

/**
 * Capacitor component implementation
 */
class Capacitor : public Component {
public:
    Capacitor(double capacitance) : capacitance_(capacitance), charge_(0.0), voltage_(0.0) {}
    
    void simulate(double timestep) override;
    double getCurrentValue() const override { return voltage_; }
    void connect(std::shared_ptr<Node> node) override;
    std::string getType() const override { return "Capacitor"; }
    
    double getCapacitance() const { return capacitance_; }

private:
    double capacitance_;
    double charge_;
    double voltage_;
};

} // namespace ic_sim