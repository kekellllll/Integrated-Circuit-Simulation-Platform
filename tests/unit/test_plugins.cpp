#include "plugins/plugin_system.h"
#include <iostream>
#include <cassert>

using namespace ic_sim;

void test_plugin_manager_singleton() {
    PluginManager& pm1 = PluginManager::getInstance();
    PluginManager& pm2 = PluginManager::getInstance();
    
    // Should be the same instance
    assert(&pm1 == &pm2);
    
    std::cout << "✓ Plugin manager singleton test passed" << std::endl;
}

void test_plugin_discovery() {
    PluginManager& pm = PluginManager::getInstance();
    
    // Test discovery of empty directory
    auto plugins = pm.discoverPlugins("nonexistent");
    assert(plugins.empty());
    
    std::cout << "✓ Plugin discovery test passed" << std::endl;
}

void test_plugin_loading() {
    PluginManager& pm = PluginManager::getInstance();
    
    // Test loading non-existent plugin
    bool result = pm.loadPlugin("nonexistent.so");
    assert(!result);
    
    auto loaded = pm.getLoadedPlugins();
    assert(loaded.empty());
    
    std::cout << "✓ Plugin loading test passed" << std::endl;
}

int main() {
    std::cout << "Running Plugin Tests..." << std::endl;
    
    try {
        test_plugin_manager_singleton();
        test_plugin_discovery();
        test_plugin_loading();
        
        std::cout << "\\n✅ All plugin tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown test failure" << std::endl;
        return 1;
    }
}