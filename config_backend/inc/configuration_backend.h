#ifndef CONFIGURATION_BACKEND_H
#define CONFIGURATION_BACKEND_H

#include <fx_chain_configuration.h>
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
    using OnGetConfigCallback = std::function<FxChainConfiguration&()>;

    virtual ~ConfigurationBackend() {}
    virtual void registerOnGetPlugins(const OnGetPluginsCallback& callback) = 0;
    virtual void registerOnApplyConfig(const OnApplyConfigCallback& callback) = 0;
    virtual void registerOnGetConfig(const OnGetConfigCallback& callback) = 0;
    virtual void start(std::uint32_t port) = 0;
};

class ConfigurationBackendImpl : public ConfigurationBackend
{
  public:
    ConfigurationBackendImpl() = default;
    ~ConfigurationBackendImpl() = default;
    void registerOnGetPlugins(const OnGetPluginsCallback& callback) override;
    void registerOnApplyConfig(const OnApplyConfigCallback& callback) override;
    void registerOnGetConfig(const OnGetConfigCallback& callback) override;
    void start(std::uint32_t port) override;

  private:
    OnGetPluginsCallback m_getPlugins;
    OnApplyConfigCallback m_applyConfig;
    OnGetConfigCallback m_getConfig;
};

}

#endif /* CONFIGURATION_BACKEND_H */
