#ifndef CONFIGURATION_BACKEND_H
#define CONFIGURATION_BACKEND_H

#include <fx_chain_configuration.h>
#include <global_settings.h>
#include <functional>
#include <cstdint>
#include <memory>

namespace awesomefx
{

class ConfigurationBackend
{
  public:
    using Ptr = std::unique_ptr<ConfigurationBackend>;
    using OnGetPluginsCallback = std::function<AvailablePlugins()>;
    using OnApplyConfigCallback = std::function<void(const FxChainConfiguration&)>;
    using OnGetConfigCallback = std::function<FxChainConfiguration()>;
    using OnSetParametersCallback = std::function<void(std::uint32_t, const std::vector<ParameterValue>&)>;
    using OnReloadCallback = std::function<void()>;
    using OnApplyGlobalSettingsCallback = std::function<void(const GlobalSettings&)>;
    using OnGetGlobalSettingsCallback = std::function<GlobalSettings()>;

    virtual ~ConfigurationBackend() {}
    virtual void registerOnGetPlugins(const OnGetPluginsCallback& callback) = 0;
    virtual void registerOnApplyConfig(const OnApplyConfigCallback& callback) = 0;
    virtual void registerOnGetConfig(const OnGetConfigCallback& callback) = 0;
    virtual void registerOnSetParameters(const OnSetParametersCallback& callback) = 0;
    virtual void registerOnReload(const OnReloadCallback& callback) = 0;
    virtual void registerOnApplyGlobalSettings(const OnApplyGlobalSettingsCallback& callback) = 0;
    virtual void registerOnGetGlobalSettings(const OnGetGlobalSettingsCallback& callback) = 0;
    virtual void start(std::uint32_t port) = 0;
};

}

#endif /* CONFIGURATION_BACKEND_H */
