#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>

namespace ic_sim {

// Forward declaration
class Component;
class Circuit;

/**
 * Plugin interface for extending simulation capabilities
 * Implements the Factory pattern for component creation
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    
    // Factory methods for creating components
    virtual std::shared_ptr<Component> createComponent(const std::string& type,
                                                      const std::map<std::string, double>& parameters) = 0;
    virtual std::vector<std::string> getSupportedComponents() const = 0;
};

/**
 * Plugin manager using the Singleton pattern
 * Handles dynamic loading and management of plugins
 */
class PluginManager {
public:
    static PluginManager& getInstance();
    
    // Plugin loading and management
    bool loadPlugin(const std::string& plugin_path);
    bool unloadPlugin(const std::string& plugin_name);
    void unloadAllPlugins();
    
    // Plugin queries
    std::vector<std::string> getLoadedPlugins() const;
    std::shared_ptr<IPlugin> getPlugin(const std::string& name);
    
    // Component creation through plugins
    std::shared_ptr<Component> createComponent(const std::string& type,
                                              const std::map<std::string, double>& parameters);
    std::vector<std::string> getAllSupportedComponents() const;
    
    // Plugin discovery
    std::vector<std::string> discoverPlugins(const std::string& directory);

private:
    PluginManager() = default;
    ~PluginManager();
    
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    std::map<std::string, std::shared_ptr<IPlugin>> plugins_;
    std::map<std::string, void*> plugin_handles_; // For dynamic library handles
};

/**
 * Base class for plugin implementations
 * Provides common functionality for plugin development
 */
class BasePlugin : public IPlugin {
public:
    BasePlugin(const std::string& name, const std::string& version, const std::string& description)
        : name_(name), version_(version), description_(description), initialized_(false) {}
    
    std::string getName() const override { return name_; }
    std::string getVersion() const override { return version_; }
    std::string getDescription() const override { return description_; }
    
    bool initialize() override {
        if (!initialized_) {
            initialized_ = doInitialize();
        }
        return initialized_;
    }
    
    void cleanup() override {
        if (initialized_) {
            doCleanup();
            initialized_ = false;
        }
    }

protected:
    virtual bool doInitialize() = 0;
    virtual void doCleanup() = 0;
    
    std::string name_;
    std::string version_;
    std::string description_;
    bool initialized_;
};

} // namespace ic_sim

// Plugin export macros for dynamic loading
#define PLUGIN_EXPORT extern "C"

// Function signature for plugin creation
typedef ic_sim::IPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(ic_sim::IPlugin*);

#define DECLARE_PLUGIN(PluginClass) \
    PLUGIN_EXPORT ic_sim::IPlugin* createPlugin() { \
        return new PluginClass(); \
    } \
    PLUGIN_EXPORT void destroyPlugin(ic_sim::IPlugin* plugin) { \
        delete plugin; \
    }
