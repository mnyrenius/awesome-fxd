#ifndef FX_PLUGIN_H
#define FX_PLUGIN_H

#include <memory>
#include <string>
#include <vector>
#include <audio_processor.h>

namespace awesomefx
{

struct FxPluginInfo
{
  std::string name;
  std::vector<std::string> parameters;
};

class FxPlugin
{
  public:
    using Ptr = std::unique_ptr<FxPlugin>;

    virtual ~FxPlugin() {}

    virtual FxPluginInfo getPluginInfo() const = 0;
    virtual AudioProcessor::Ptr createAudioProcessor() const = 0;
};

}

#endif /* FX_PLUGIN_H */
