/**
 * Simulation Interface Module
 * Handles communication between the web UI and the C++ simulation backend
 */

class SimulationInterface {
    constructor() {
        this.isConnected = false;
        this.simulationWorker = null;
        this.currentCircuit = null;
        this.simulationResults = [];
        
        this.init();
    }
    
    init() {
        // Initialize WebAssembly interface (placeholder)
        this.initWebAssembly();
        
        // Set up simulation worker for background processing
        this.setupSimulationWorker();
        
        console.log('Simulation interface initialized');
    }
    
    async initWebAssembly() {
        try {
            // In a real implementation, this would load the WebAssembly module
            // compiled from the C++/CUDA simulation engine
            console.log('Initializing WebAssembly simulation engine...');
            
            // Simulate loading delay
            await new Promise(resolve => setTimeout(resolve, 1000));
            
            this.isConnected = true;
            this.updateStatus('WebAssembly engine ready');
            
        } catch (error) {
            console.error('Failed to initialize WebAssembly:', error);
            this.updateStatus('Using JavaScript fallback simulation');
            this.isConnected = false;
        }
    }
    
    setupSimulationWorker() {
        // Create inline worker for simulation calculations
        const workerCode = `
            class CircuitSimulator {
                constructor() {
                    this.components = new Map();
                    this.nodes = new Map();
                    this.timeStep = 0.001;
                }
                
                addComponent(id, type, parameters) {
                    this.components.set(id, {
                        id, type, parameters,
                        voltage: 0,
                        current: 0
                    });
                }
                
                addNode(id, voltage = 0) {
                    this.nodes.set(id, { id, voltage, connections: [] });
                }
                
                simulate(duration, timeStep) {
                    const results = [];
                    const steps = Math.floor(duration / timeStep);
                    
                    for (let step = 0; step < steps; step++) {
                        const time = step * timeStep;
                        
                        // Simple circuit analysis (placeholder)
                        this.components.forEach((component, id) => {
                            switch (component.type) {
                                case 'resistor':
                                    // Ohm's law: I = V/R
                                    component.current = component.voltage / component.parameters.resistance;
                                    break;
                                    
                                case 'capacitor':
                                    // Capacitor charging: V = V0 * (1 - e^(-t/RC))
                                    const tau = component.parameters.capacitance * 1000; // Assume 1k ohm
                                    component.voltage = 5 * (1 - Math.exp(-time / tau));
                                    component.current = (5 / 1000) * Math.exp(-time / tau);
                                    break;
                                    
                                case 'voltage-source':
                                    component.voltage = component.parameters.voltage;
                                    component.current = 0.01; // Assume small internal resistance
                                    break;
                            }
                        });
                        
                        // Collect results every 10 steps
                        if (step % 10 === 0) {
                            const snapshot = {
                                time: time,
                                components: Array.from(this.components.entries()).map(([id, comp]) => ({
                                    id: id,
                                    voltage: comp.voltage,
                                    current: comp.current,
                                    power: comp.voltage * comp.current
                                }))
                            };
                            results.push(snapshot);
                        }
                    }
                    
                    return results;
                }
            }
            
            const simulator = new CircuitSimulator();
            
            self.onmessage = function(e) {
                const { type, data } = e.data;
                
                switch (type) {
                    case 'addComponent':
                        simulator.addComponent(data.id, data.type, data.parameters);
                        self.postMessage({ type: 'componentAdded', data: data.id });
                        break;
                        
                    case 'simulate':
                        const results = simulator.simulate(data.duration, data.timeStep);
                        self.postMessage({ type: 'simulationComplete', data: results });
                        break;
                        
                    case 'reset':
                        simulator.components.clear();
                        simulator.nodes.clear();
                        self.postMessage({ type: 'resetComplete' });
                        break;
                }
            };
        `;
        
        const blob = new Blob([workerCode], { type: 'application/javascript' });
        this.simulationWorker = new Worker(URL.createObjectURL(blob));
        
        this.simulationWorker.onmessage = (e) => {
            this.handleWorkerMessage(e.data);
        };
        
        console.log('Simulation worker created');
    }
    
    handleWorkerMessage(message) {
        const { type, data } = message;
        
        switch (type) {
            case 'componentAdded':
                console.log(`Component added to simulation: ${data}`);
                break;
                
            case 'simulationComplete':
                this.simulationResults = data;
                this.updateVisualization(data);
                this.updateStatus(`Simulation complete: ${data.length} data points`);
                break;
                
            case 'resetComplete':
                this.simulationResults = [];
                this.updateStatus('Simulation reset');
                break;
        }
    }
    
    addComponent(type, id, parameters) {
        // Add to worker simulation
        this.simulationWorker.postMessage({
            type: 'addComponent',
            data: { id, type, parameters }
        });
        
        // Add to visualization
        const position = this.getNextComponentPosition();
        circuitViz.addComponent(type, id, position, parameters);
        circuitViz.updateComponentCount();
    }
    
    startSimulation(duration, timeStep) {
        if (!this.simulationWorker) {
            this.updateStatus('Simulation worker not available');
            return;
        }
        
        this.updateStatus('Starting simulation...');
        
        // Start visualization animation
        circuitViz.startSimulation();
        
        // Start worker simulation
        this.simulationWorker.postMessage({
            type: 'simulate',
            data: { duration, timeStep }
        });
    }
    
    pauseSimulation() {
        circuitViz.pauseSimulation();
        this.updateStatus('Simulation paused');
    }
    
    resetSimulation() {
        this.simulationWorker.postMessage({ type: 'reset' });
        circuitViz.resetSimulation();
        this.simulationResults = [];
    }
    
    updateVisualization(results) {
        if (results.length === 0) return;
        
        // Animate through simulation results
        let resultIndex = 0;
        const animateResults = () => {
            if (resultIndex >= results.length) {
                this.updateStatus('Visualization complete');
                return;
            }
            
            const snapshot = results[resultIndex];
            
            // Update component visuals based on simulation data
            snapshot.components.forEach(comp => {
                circuitViz.updateComponentValues(comp.id, {
                    voltage: comp.voltage,
                    current: comp.current,
                    power: comp.power
                });
            });
            
            resultIndex++;
            setTimeout(animateResults, 100); // Update every 100ms
        };
        
        animateResults();
    }
    
    getNextComponentPosition() {
        const componentCount = circuitViz.components.size;
        const radius = 5;
        const angle = (componentCount * 60) % 360; // Spread components in circle
        
        return {
            x: radius * Math.cos(angle * Math.PI / 180),
            y: 1,
            z: radius * Math.sin(angle * Math.PI / 180)
        };
    }
    
    exportResults() {
        if (this.simulationResults.length === 0) {
            this.updateStatus('No simulation results to export');
            return;
        }
        
        // Convert results to CSV format
        const csv = this.convertToCSV(this.simulationResults);
        
        // Download as file
        const blob = new Blob([csv], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `simulation_results_${new Date().toISOString()}.csv`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        this.updateStatus('Results exported to CSV');
    }
    
    convertToCSV(results) {
        if (results.length === 0) return '';
        
        // Header
        const headers = ['Time'];
        const firstSnapshot = results[0];
        firstSnapshot.components.forEach(comp => {
            headers.push(`${comp.id}_Voltage`, `${comp.id}_Current`, `${comp.id}_Power`);
        });
        
        let csv = headers.join(',') + '\\n';
        
        // Data rows
        results.forEach(snapshot => {
            const row = [snapshot.time];
            snapshot.components.forEach(comp => {
                row.push(comp.voltage, comp.current, comp.power);
            });
            csv += row.join(',') + '\\n';
        });
        
        return csv;
    }
    
    loadCircuit() {
        // Create file input for loading circuit definitions
        const input = document.createElement('input');
        input.type = 'file';
        input.accept = '.json,.circuit';
        
        input.onchange = (e) => {
            const file = e.target.files[0];
            if (!file) return;
            
            const reader = new FileReader();
            reader.onload = (e) => {
                try {
                    const circuitData = JSON.parse(e.target.result);
                    this.loadCircuitFromData(circuitData);
                } catch (error) {
                    console.error('Failed to load circuit:', error);
                    this.updateStatus('Failed to load circuit file');
                }
            };
            reader.readAsText(file);
        };
        
        input.click();
    }
    
    loadCircuitFromData(circuitData) {
        // Reset current circuit
        this.resetSimulation();
        
        // Load components
        if (circuitData.components) {
            circuitData.components.forEach(comp => {
                this.addComponent(comp.type, comp.id, comp.parameters);
            });
        }
        
        // Load connections
        if (circuitData.connections) {
            circuitData.connections.forEach(conn => {
                circuitViz.addConnection(conn.from, conn.to, conn.id);
            });
        }
        
        this.updateStatus(`Loaded circuit: ${circuitData.name || 'Unnamed'}`);
    }
    
    updateStatus(message) {
        const statusElement = document.getElementById('status');
        if (statusElement) {
            statusElement.textContent = message;
        }
        console.log('Status:', message);
    }
}

// Global simulation interface
let simInterface;

// UI event handlers
function addComponent() {
    const typeSelect = document.getElementById('component-type');
    const type = typeSelect.value;
    const id = `${type}_${Date.now()}`;
    
    // Default parameters based on component type
    const parameters = {
        'resistor': { resistance: 1000 },
        'capacitor': { capacitance: 1e-6 },
        'inductor': { inductance: 1e-3 },
        'voltage-source': { voltage: 5 }
    };
    
    simInterface.addComponent(type, id, parameters[type] || {});
}

function startSimulation() {
    const duration = parseFloat(document.getElementById('duration').value);
    const timestep = parseFloat(document.getElementById('timestep').value) / 1000; // Convert to seconds
    
    simInterface.startSimulation(duration, timestep);
}

function pauseSimulation() {
    simInterface.pauseSimulation();
}

function resetSimulation() {
    simInterface.resetSimulation();
}

function changeViewMode() {
    const viewMode = document.getElementById('view-mode').value;
    console.log('View mode changed to:', viewMode);
    // Implementation would switch between 3D, schematic, and analysis views
}

function exportResults() {
    simInterface.exportResults();
}

function loadCircuit() {
    simInterface.loadCircuit();
}

// Initialize simulation interface when page loads
document.addEventListener('DOMContentLoaded', () => {
    simInterface = new SimulationInterface();
});