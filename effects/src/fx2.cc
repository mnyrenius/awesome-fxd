#include <audio_processor.h>
#include <fx_plugin.h>
#include <cstring>
#include <memory>

namespace awesomefx
{

class SimpleDistortion : public AudioProcessor
{
  public:
    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      std::memcpy(out_l, in_l, sizeof(Sample) * numSamples);
      std::memcpy(out_r, in_r, sizeof(Sample) * numSamples);
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
    }
};

class SimpleDistortionPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"SimpleDistortion", {"Param1", "Param2"}};
    }

    AudioProcessor::Ptr createAudioProcessor() const override
    {
      return std::make_unique<SimpleDistortion>();
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<SimpleDistortionPlugin>();
}

