#include "plugins/plugin_system.h"
#include "core/circuit.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(path) LoadLibraryA(path)
#define GET_PROC_ADDRESS(handle, name) GetProcAddress((HMODULE)handle, name)
#define CLOSE_LIBRARY(handle) FreeLibrary((HMODULE)handle)
#define LIBRARY_EXTENSION ".dll"
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
#define CLOSE_LIBRARY(handle) dlclose(handle)
#define LIBRARY_EXTENSION ".so"
#endif

namespace ic_sim {

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

bool PluginManager::loadPlugin(const std::string& plugin_path) {
    std::cout << "Loading plugin: " << plugin_path << std::endl;
    
    // Load the dynamic library
    void* handle = LOAD_LIBRARY(plugin_path.c_str());
    if (!handle) {
        std::cerr << "Failed to load plugin library: " << plugin_path << std::endl;
        return false;
    }
    
    // Get the plugin creation function
    CreatePluginFunc createPlugin = (CreatePluginFunc)GET_PROC_ADDRESS(handle, "createPlugin");
    if (!createPlugin) {
        std::cerr << "Plugin does not export createPlugin function: " << plugin_path << std::endl;
        CLOSE_LIBRARY(handle);
        return false;
    }
    
    // Create the plugin instance
    std::shared_ptr<IPlugin> plugin(createPlugin());
    if (!plugin) {
        std::cerr << "Failed to create plugin instance: " << plugin_path << std::endl;
        CLOSE_LIBRARY(handle);
        return false;
    }
    
    // Initialize the plugin
    if (!plugin->initialize()) {
        std::cerr << "Failed to initialize plugin: " << plugin->getName() << std::endl;
        CLOSE_LIBRARY(handle);
        return false;
    }
    
    // Store the plugin
    std::string plugin_name = plugin->getName();
    plugins_[plugin_name] = plugin;
    plugin_handles_[plugin_name] = handle;
    
    std::cout << "Successfully loaded plugin: " << plugin_name 
              << " v" << plugin->getVersion() << std::endl;
    
    return true;
}

bool PluginManager::unloadPlugin(const std::string& plugin_name) {
    auto plugin_it = plugins_.find(plugin_name);
    if (plugin_it == plugins_.end()) {
        return false;
    }
    
    // Cleanup the plugin
    plugin_it->second->cleanup();
    
    // Remove from maps
    plugins_.erase(plugin_it);
    
    auto handle_it = plugin_handles_.find(plugin_name);
    if (handle_it != plugin_handles_.end()) {
        CLOSE_LIBRARY(handle_it->second);
        plugin_handles_.erase(handle_it);
    }
    
    std::cout << "Unloaded plugin: " << plugin_name << std::endl;
    return true;
}

void PluginManager::unloadAllPlugins() {
    std::vector<std::string> plugin_names;
    for (const auto& [name, plugin] : plugins_) {
        plugin_names.push_back(name);
    }
    
    for (const auto& name : plugin_names) {
        unloadPlugin(name);
    }
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
    std::vector<std::string> names;
    for (const auto& [name, plugin] : plugins_) {
        names.push_back(name);
    }
    return names;
}

std::shared_ptr<IPlugin> PluginManager::getPlugin(const std::string& name) {
    auto it = plugins_.find(name);
    return (it != plugins_.end()) ? it->second : nullptr;
}

std::shared_ptr<Component> PluginManager::createComponent(const std::string& type,
                                                         const std::map<std::string, double>& parameters) {
    // Try each loaded plugin until one can create the component
    for (const auto& [name, plugin] : plugins_) {
        auto supported = plugin->getSupportedComponents();
        if (std::find(supported.begin(), supported.end(), type) != supported.end()) {
            auto component = plugin->createComponent(type, parameters);
            if (component) {
                std::cout << "Created component '" << type << "' using plugin '" << name << "'" << std::endl;
                return component;
            }
        }
    }
    
    std::cerr << "No plugin found to create component type: " << type << std::endl;
    return nullptr;
}

std::vector<std::string> PluginManager::getAllSupportedComponents() const {
    std::vector<std::string> all_components;
    
    for (const auto& [name, plugin] : plugins_) {
        auto supported = plugin->getSupportedComponents();
        all_components.insert(all_components.end(), supported.begin(), supported.end());
    }
    
    // Remove duplicates
    std::sort(all_components.begin(), all_components.end());
    all_components.erase(std::unique(all_components.begin(), all_components.end()), 
                        all_components.end());
    
    return all_components;
}

std::vector<std::string> PluginManager::discoverPlugins(const std::string& directory) {
    std::vector<std::string> discovered_plugins;
    
    try {
        if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find(LIBRARY_EXTENSION) != std::string::npos) {
                        discovered_plugins.push_back(entry.path().string());
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error discovering plugins in " << directory << ": " << e.what() << std::endl;
    }
    
    return discovered_plugins;
}

} // namespace ic_sim