#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "fx_plugin_handler.h"
#include "jack_client.h"
#include <configuration_backend.h>
#include <fx_chain_configuration.h>
#include <global_settings.h>

namespace awesomefx
{

class Controller
{
  public:
    virtual ~Controller() {}
    virtual void start() = 0;
};

class ControllerImpl : public Controller
{
  public:
    ControllerImpl(
        std::vector<std::string> inputs,
        FxPluginHandler::Factory pluginHandlerFactory,
        JackClient::Factory jackClientFactory,
        ConfigurationBackend::Ptr configBackend);

    ControllerImpl() = delete;
    ~ControllerImpl() override = default;
    void start() override;

  private:
    std::vector<std::string> m_inputs;
    FxPluginHandler::Factory m_pluginHandlerFactory;
    FxPluginHandler::Ptr m_pluginHandler;
    JackClient::Factory m_jackClientFactory;
    ConfigurationBackend::Ptr m_configBackend;
    std::vector<JackClient::Ptr> m_fxChain;
    FxChainConfiguration m_currentConfig;
    GlobalSettings m_globalSettings;
};

}

#endif /* CONTROLLER_H */
