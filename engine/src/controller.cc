#include "controller.h"
#include <fx_chain_configuration.h>
#include <algorithm>

using namespace awesomefx;

ControllerImpl::ControllerImpl(
        std::vector<std::string> inputs,
        FxPluginHandler::Factory pluginHandlerFactory,
        JackClient::Factory jackClientFactory,
        ConfigurationBackend::Ptr configBackend)
  :
    m_inputs(std::move(inputs)),
    m_pluginHandlerFactory(std::move(pluginHandlerFactory)),
    m_jackClientFactory(std::move(jackClientFactory)),
    m_configBackend(std::move(configBackend))
{
  m_pluginHandler = m_pluginHandlerFactory();
}

void ControllerImpl::start()
{
  auto onApplyConfig = [this](const FxChainConfiguration& config) {
    m_fxChain.clear();
    m_currentConfig = config;

    if (config.empty())
    {
      return;
    }

    for (auto i = 0U; i < config.size(); ++i)
    {
      auto& effect = config[i];
      auto& plugin = m_pluginHandler->getPlugin(effect.name);

      auto processorFactory = [&plugin] (auto& context) {
        return plugin.createAudioProcessor(context);
      };

      auto client = m_jackClientFactory(effect.name, processorFactory);

      for (auto param = 0U; param < effect.parameters.size(); ++param)
      {
        client->setParameter({param, effect.parameters[param]});
      }

      if (i > 0)
      {
        client->connectInputs(m_fxChain[i-1]->getOutputPorts());
      }
      m_fxChain.push_back(std::move(client));
    }

    m_fxChain.front()->connectInputsToCapturePorts(m_inputs, m_globalSettings.monoInput);
    m_fxChain.back()->connectOutputsToPlaybackPorts();
  };

  m_configBackend->registerOnApplyConfig(onApplyConfig);

  auto onGetPlugins = [this]() {
    AvailablePlugins plugins;
    auto allPlugins = m_pluginHandler->getAllPlugins();
    std::transform(
        allPlugins.begin(),
        allPlugins.end(),
        std::back_inserter(plugins),
        [](auto& plugin) {
        auto info = plugin->getPluginInfo();
        return AvailablePlugin { info.name, info.parameters };
        });
    return plugins;
  };

  m_configBackend->registerOnGetPlugins(onGetPlugins);

  auto onGetConfig = [this] () {
    return m_currentConfig;
  };

  m_configBackend->registerOnGetConfig(onGetConfig);

  auto onSetParameters = [this] (std::uint32_t index, const std::vector<ParameterValue>& params) {
    if (index >= m_fxChain.size())
    {
      return;
    }

    auto& fx = m_fxChain[index];
    for (auto i = 0U; i < params.size(); ++i)
    {
      fx->setParameter({i, params[i]});
    }
  };

  m_configBackend->registerOnSetParameters(onSetParameters);

  auto onReload = [=] {
    m_fxChain.clear();
    m_pluginHandler.reset();
    m_pluginHandler = m_pluginHandlerFactory();
    onApplyConfig(m_currentConfig);
  };

  m_configBackend->registerOnReload(onReload);

  auto onApplyGlobalSettings = [=](const GlobalSettings& settings) {
    m_globalSettings = settings;
    onApplyConfig(m_currentConfig);
  };

  m_configBackend->registerOnApplyGlobalSettings(onApplyGlobalSettings);

  auto onGetGlobalSettings = [this] {
    return m_globalSettings;
  };

  m_configBackend->registerOnGetGlobalSettings(onGetGlobalSettings);

  printf("Available plugins:\n\n");
  for (auto plugin : onGetPlugins())
  {
    printf("Plugin: %s\n", plugin.name.c_str());
    for (auto& param : plugin.parameters)
    {
      printf("\tParameter: %s\n", param.c_str());
    }
  }

  FxChainConfiguration conf
  {
    {
      "Passthrough",
        { 1, 2 }
    }
  };

  onApplyConfig(conf);

  printf("\n\nCurrent configuration:\n\n");
  for (auto plugin : onGetConfig())
  {
    printf("Plugin: %s\n", plugin.name.c_str());
    for (auto& param : plugin.parameters)
    {
      printf("\tParameter: %f\n", param);
    }
  }
}

