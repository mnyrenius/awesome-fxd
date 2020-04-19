#ifndef FX_PLUGIN_HANDLER_H
#define FX_PLUGIN_HANDLER_H

#include <map>
#include <memory>
#include <fx_plugin.h>

namespace awesomefx
{

using FxPluginMap = std::map<std::string, FxPlugin::Ptr>;

class FxPluginHandler
{
  public:
    using Ptr = std::unique_ptr<FxPluginHandler>;

    virtual ~FxPluginHandler() {}
    virtual std::vector<const FxPlugin *> getAllPlugins() const = 0;
    virtual const FxPlugin& getPlugin(const std::string& name) const= 0;
};


class FxPluginHandlerImpl : public FxPluginHandler
{
  public:
    FxPluginHandlerImpl(const std::string& dir);
    FxPluginHandlerImpl() = delete;
    FxPluginHandlerImpl(const FxPluginHandlerImpl&) = delete;
    FxPluginHandlerImpl(const FxPluginHandlerImpl&&) = delete;
    ~FxPluginHandlerImpl() override;

    const FxPlugin& getPlugin(const std::string& name) const override;
    std::vector<const FxPlugin *> getAllPlugins() const override;

  private:
    FxPluginMap m_fxPlugins;
    std::vector<void *> m_handles;
};

}

#endif /* FX_PLUGIN_HANDLER_H */
