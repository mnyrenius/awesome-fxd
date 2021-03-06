#ifndef FX_CHAIN_CONFIGURATION_H
#define FX_CHAIN_CONFIGURATION_H

#include <string>
#include <vector>

namespace awesomefx
{
using ParameterValue = float;

struct FxConfiguration
{
  std::string name;
  std::vector<ParameterValue> parameters;
};

using FxChainConfiguration = std::vector<FxConfiguration>;

struct AvailablePlugin
{
  std::string name;
  std::vector<std::string> parameters;
};

using AvailablePlugins = std::vector<AvailablePlugin>;

}

#endif /* FX_CHAIN_CONFIGURATION_H */
