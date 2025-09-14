/**
 * Circuit Visualization Module
 * Provides 3D rendering of circuit components and connections using Three.js
 */

class CircuitVisualization {
    constructor(containerId) {
        this.container = document.getElementById(containerId);
        this.scene = null;
        this.camera = null;
        this.renderer = null;
        this.controls = null;
        this.components = new Map();
        this.connections = new Map();
        this.isSimulating = false;
        this.animationId = null;
        
        this.init();
    }
    
    init() {
        // Create scene
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x0a0a0a);
        
        // Create camera
        this.camera = new THREE.PerspectiveCamera(
            75, 
            window.innerWidth / window.innerHeight, 
            0.1, 
            1000
        );
        this.camera.position.set(10, 10, 10);
        
        // Create renderer with WebGL
        this.renderer = new THREE.WebGLRenderer({ 
            antialias: true,
            alpha: true 
        });
        this.renderer.setSize(window.innerWidth, window.innerHeight);
        this.renderer.shadowMap.enabled = true;
        this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        this.container.appendChild(this.renderer.domElement);
        
        // Add orbit controls
        this.controls = new THREE.OrbitControls(this.camera, this.renderer.domElement);
        this.controls.enableDamping = true;
        this.controls.dampingFactor = 0.1;
        
        // Add lighting
        this.setupLighting();
        
        // Add grid and axes
        this.addGrid();
        this.addAxes();
        
        // Create default circuit
        this.createDemoCircuit();
        
        // Start render loop
        this.animate();
        
        // Handle window resize
        window.addEventListener('resize', () => this.onWindowResize());
        
        console.log('Circuit visualization initialized');
    }
    
    setupLighting() {
        // Ambient light
        const ambientLight = new THREE.AmbientLight(0x404040, 0.3);
        this.scene.add(ambientLight);
        
        // Directional light
        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(10, 10, 5);
        directionalLight.castShadow = true;
        directionalLight.shadow.mapSize.width = 2048;
        directionalLight.shadow.mapSize.height = 2048;
        this.scene.add(directionalLight);
        
        // Point lights for circuit board effect
        const pointLight1 = new THREE.PointLight(0x00ff00, 0.5, 20);
        pointLight1.position.set(5, 2, 5);
        this.scene.add(pointLight1);
        
        const pointLight2 = new THREE.PointLight(0x0088ff, 0.5, 20);
        pointLight2.position.set(-5, 2, -5);
        this.scene.add(pointLight2);
    }
    
    addGrid() {
        const gridHelper = new THREE.GridHelper(20, 20, 0x333333, 0x333333);
        this.scene.add(gridHelper);
        
        // Add circuit board texture
        const boardGeometry = new THREE.PlaneGeometry(20, 20);
        const boardMaterial = new THREE.MeshLambertMaterial({ 
            color: 0x004400,
            transparent: true,
            opacity: 0.3
        });
        const board = new THREE.Mesh(boardGeometry, boardMaterial);
        board.rotation.x = -Math.PI / 2;
        board.position.y = -0.01;
        this.scene.add(board);
    }
    
    addAxes() {
        const axesHelper = new THREE.AxesHelper(5);
        this.scene.add(axesHelper);
    }
    
    createDemoCircuit() {
        // Create a simple RC circuit as demo
        this.addComponent('resistor', 'R1', { x: -3, y: 1, z: 0 }, { resistance: 1000 });
        this.addComponent('capacitor', 'C1', { x: 0, y: 1, z: 0 }, { capacitance: 1e-6 });
        this.addComponent('voltage-source', 'V1', { x: 3, y: 1, z: 0 }, { voltage: 5 });
        
        // Add connections
        this.addConnection('R1', 'C1', 'wire1');
        this.addConnection('C1', 'V1', 'wire2');
        
        this.updateComponentCount();
    }
    
    addComponent(type, id, position, parameters) {
        let geometry, material, mesh;
        
        switch (type) {
            case 'resistor':
                geometry = new THREE.CylinderGeometry(0.1, 0.1, 1.5, 8);
                material = new THREE.MeshPhongMaterial({ 
                    color: 0xcc6600,
                    shininess: 30
                });
                mesh = new THREE.Mesh(geometry, material);
                mesh.rotation.z = Math.PI / 2;
                
                // Add resistor bands for visual effect
                for (let i = 0; i < 4; i++) {
                    const bandGeometry = new THREE.CylinderGeometry(0.11, 0.11, 0.1, 8);
                    const colors = [0xff0000, 0x00ff00, 0x0000ff, 0xffff00];
                    const bandMaterial = new THREE.MeshPhongMaterial({ color: colors[i] });
                    const band = new THREE.Mesh(bandGeometry, bandMaterial);
                    band.position.x = -0.5 + i * 0.3;
                    band.rotation.z = Math.PI / 2;
                    mesh.add(band);
                }
                break;
                
            case 'capacitor':
                const group = new THREE.Group();
                
                // Two plates
                const plateGeometry = new THREE.BoxGeometry(0.1, 1, 1);
                const plateMaterial = new THREE.MeshPhongMaterial({ color: 0x888888 });
                
                const plate1 = new THREE.Mesh(plateGeometry, plateMaterial);
                plate1.position.x = -0.2;
                group.add(plate1);
                
                const plate2 = new THREE.Mesh(plateGeometry, plateMaterial);
                plate2.position.x = 0.2;
                group.add(plate2);
                
                mesh = group;
                break;
                
            case 'inductor':
                // Create coil shape
                const curve = new THREE.CatmullRomCurve3([
                    new THREE.Vector3(-0.75, 0, 0),
                    new THREE.Vector3(-0.5, 0.3, 0),
                    new THREE.Vector3(-0.25, -0.3, 0),
                    new THREE.Vector3(0, 0.3, 0),
                    new THREE.Vector3(0.25, -0.3, 0),
                    new THREE.Vector3(0.5, 0.3, 0),
                    new THREE.Vector3(0.75, 0, 0)
                ]);
                
                const coilGeometry = new THREE.TubeGeometry(curve, 20, 0.05, 8, false);
                const coilMaterial = new THREE.MeshPhongMaterial({ color: 0xcc8800 });
                mesh = new THREE.Mesh(coilGeometry, coilMaterial);
                break;
                
            case 'voltage-source':
                geometry = new THREE.CylinderGeometry(0.4, 0.4, 0.2, 16);
                material = new THREE.MeshPhongMaterial({ 
                    color: 0x006600,
                    shininess: 50
                });
                mesh = new THREE.Mesh(geometry, material);
                
                // Add + and - symbols
                const textGeometry = new THREE.RingGeometry(0.15, 0.25, 8);
                const textMaterial = new THREE.MeshBasicMaterial({ color: 0xffffff });
                const plus = new THREE.Mesh(textGeometry, textMaterial);
                plus.position.y = 0.11;
                mesh.add(plus);
                break;
                
            default:
                geometry = new THREE.BoxGeometry(0.5, 0.5, 0.5);
                material = new THREE.MeshPhongMaterial({ color: 0xff0000 });
                mesh = new THREE.Mesh(geometry, material);
        }
        
        mesh.position.set(position.x, position.y, position.z);
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        mesh.userData = { id, type, parameters };
        
        this.scene.add(mesh);
        this.components.set(id, mesh);
        
        // Add label
        this.addComponentLabel(mesh, id);
        
        console.log(`Added ${type} component: ${id}`);
    }
    
    addComponentLabel(component, text) {
        const canvas = document.createElement('canvas');
        const context = canvas.getContext('2d');
        canvas.width = 128;
        canvas.height = 32;
        
        context.fillStyle = 'rgba(0, 0, 0, 0.8)';
        context.fillRect(0, 0, canvas.width, canvas.height);
        
        context.fillStyle = 'white';
        context.font = '16px Arial';
        context.textAlign = 'center';
        context.fillText(text, canvas.width / 2, canvas.height / 2 + 6);
        
        const texture = new THREE.CanvasTexture(canvas);
        const spriteMaterial = new THREE.SpriteMaterial({ map: texture });
        const sprite = new THREE.Sprite(spriteMaterial);
        sprite.position.y = 1;
        sprite.scale.set(2, 0.5, 1);
        
        component.add(sprite);
    }
    
    addConnection(fromId, toId, connectionId) {
        const fromComponent = this.components.get(fromId);
        const toComponent = this.components.get(toId);
        
        if (!fromComponent || !toComponent) {
            console.error(`Cannot create connection: component not found`);
            return;
        }
        
        const points = [
            fromComponent.position.clone(),
            toComponent.position.clone()
        ];
        
        const geometry = new THREE.BufferGeometry().setFromPoints(points);
        const material = new THREE.LineBasicMaterial({ 
            color: 0x00ff00, 
            linewidth: 3 
        });
        const line = new THREE.Line(geometry, material);
        
        this.scene.add(line);
        this.connections.set(connectionId, line);
        
        console.log(`Added connection: ${fromId} -> ${toId}`);
    }
    
    updateComponentValues(componentId, values) {
        const component = this.components.get(componentId);
        if (!component) return;
        
        // Update component color based on simulation values
        const material = component.material || component.children[0]?.material;
        if (material) {
            if (values.current > 0.1) {
                material.color.setHex(0xff4444); // Red for high current
            } else if (values.voltage > 2) {
                material.color.setHex(0x44ff44); // Green for high voltage
            } else {
                material.color.setHex(0x4444ff); // Blue for low values
            }
            material.needsUpdate = true;
        }
    }
    
    startSimulation() {
        this.isSimulating = true;
        console.log('Simulation started');
        
        // Animate current flow
        this.animateCurrentFlow();
    }
    
    pauseSimulation() {
        this.isSimulating = false;
        console.log('Simulation paused');
    }
    
    resetSimulation() {
        this.isSimulating = false;
        
        // Reset all component colors
        this.components.forEach((component) => {
            const material = component.material || component.children[0]?.material;
            if (material) {
                material.color.setHex(0x888888);
                material.needsUpdate = true;
            }
        });
        
        console.log('Simulation reset');
    }
    
    animateCurrentFlow() {
        if (!this.isSimulating) return;
        
        // Simple animation of current flow through connections
        this.connections.forEach((connection, id) => {
            const time = Date.now() * 0.005;
            const intensity = (Math.sin(time) + 1) * 0.5;
            
            if (connection.material) {
                connection.material.color.setRGB(0, intensity, 0);
                connection.material.needsUpdate = true;
            }
        });
        
        setTimeout(() => this.animateCurrentFlow(), 100);
    }
    
    updateComponentCount() {
        const componentCount = this.components.size;
        const nodeCount = this.connections.size + 1; // Approximate
        
        document.getElementById('component-count').textContent = componentCount;
        document.getElementById('node-count').textContent = nodeCount;
    }
    
    animate() {
        this.animationId = requestAnimationFrame(() => this.animate());
        
        this.controls.update();
        this.renderer.render(this.scene, this.camera);
    }
    
    onWindowResize() {
        this.camera.aspect = window.innerWidth / window.innerHeight;
        this.camera.updateProjectionMatrix();
        this.renderer.setSize(window.innerWidth, window.innerHeight);
    }
    
    dispose() {
        if (this.animationId) {
            cancelAnimationFrame(this.animationId);
        }
        
        this.renderer.dispose();
        this.scene.clear();
        
        console.log('Circuit visualization disposed');
    }
}

// Global visualization instance
let circuitViz;

// Initialize when page loads
document.addEventListener('DOMContentLoaded', () => {
    circuitViz = new CircuitVisualization('canvas-container');
});