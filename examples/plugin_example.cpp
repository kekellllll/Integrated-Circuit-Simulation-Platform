#include "plugins/plugin_system.h"
#include "core/circuit.h"
#include <iostream>

using namespace ic_sim;

int main() {
    std::cout << "Plugin Example" << std::endl;
    std::cout << "===============" << std::endl;
    
    PluginManager& pm = PluginManager::getInstance();
    
    // Discover plugins in the current directory
    std::cout << "Discovering plugins..." << std::endl;
    auto plugins = pm.discoverPlugins("./lib/plugins/");
    
    std::cout << "Found " << plugins.size() << " plugin(s):" << std::endl;
    for (const auto& plugin : plugins) {
        std::cout << "  - " << plugin << std::endl;
    }
    
    // Try to load plugins
    for (const auto& plugin_path : plugins) {
        std::cout << "Loading plugin: " << plugin_path << std::endl;
        if (pm.loadPlugin(plugin_path)) {
            std::cout << "  ✓ Successfully loaded" << std::endl;
        } else {
            std::cout << "  ✗ Failed to load" << std::endl;
        }
    }
    
    // List loaded plugins
    auto loaded = pm.getLoadedPlugins();
    std::cout << "\\nLoaded plugins:" << std::endl;
    for (const auto& name : loaded) {
        auto plugin = pm.getPlugin(name);
        if (plugin) {
            std::cout << "  - " << plugin->getName() 
                     << " v" << plugin->getVersion() 
                     << ": " << plugin->getDescription() << std::endl;
        }
    }
    
    // List supported components
    auto supported = pm.getAllSupportedComponents();
    std::cout << "\\nSupported component types:" << std::endl;
    for (const auto& type : supported) {
        std::cout << "  - " << type << std::endl;
    }
    
    // Try to create components through plugins
    std::cout << "\\nTesting component creation..." << std::endl;
    
    auto inductor = pm.createComponent("Inductor", {{"inductance", 1e-3}});
    if (inductor) {
        std::cout << "✓ Created " << inductor->getType() << " component" << std::endl;
    }
    
    auto diode = pm.createComponent("Diode", {{"forward_voltage", 0.7}});
    if (diode) {
        std::cout << "✓ Created " << diode->getType() << " component" << std::endl;
    }
    
    std::cout << "\\nPlugin example completed!" << std::endl;
    
    return 0;
}