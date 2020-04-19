#include "controller.h"
#include <fx_chain_configuration.h>

using namespace awesomefx;

ControllerImpl::ControllerImpl(
        std::vector<std::string> inputs,
        FxPluginHandler::Ptr pluginHandler,
        JackClient::Factory jackClientFactory,
        ConfigurationBackend::Ptr configBackend)
  :
    m_inputs(std::move(inputs)),
    m_pluginHandler(std::move(pluginHandler)),
    m_jackClientFactory(std::move(jackClientFactory)),
    m_configBackend(std::move(configBackend))
{
  auto onApplyConfig = [this](const FxChainConfiguration& config) {
    m_fxChain.clear();

    for (auto i = 0U; i < config.size(); ++i)
    {
      auto& effect = config[i];
      auto& plugin = m_pluginHandler->getPlugin(effect.name);
      auto processor = plugin.createAudioProcessor();

      for (auto param = 0U; param < effect.parameters.size(); ++param)
      {
        processor->setParameter({param, effect.parameters[param]});
      }

      auto client = m_jackClientFactory(effect.name, std::move(processor));
      if (i > 0)
      {
        client->connectInputs(m_fxChain[i-1]->getOutputPorts());
      }
      m_fxChain.push_back(std::move(client));
    }

    m_fxChain.front()->connectInputsToCapturePorts(m_inputs);
    m_fxChain.back()->connectOutputsToPlaybackPorts();

    m_currentConfig = config;
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
    },
      {
        "Passthrough",
        { 1, 2 }
      },
      {
        "SimpleDistortion",
        { 3, 4 }
      }
  };

  onApplyConfig(conf);

  printf("\n\nCurrent configuration:\n\n");
  for (auto plugin : onGetConfig())
  {
    printf("Plugin: %s\n", plugin.name.c_str());
    for (auto& param : plugin.parameters)
    {
      printf("\tParameter: %d\n", param);
    }
  }
}

