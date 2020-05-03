#include <audio_processor.h>
#include <fx_plugin.h>
#include <cstring>
#include <memory>

namespace awesomefx
{

class Passthrough : public AudioProcessor
{
  public:
    Passthrough(const AudioProcessingContext& context)
    {
      printf("Current sample rate: %u\n", context.getSampleRate());
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      std::memcpy(out_l, in_l, sizeof(Sample) * numSamples);
      std::memcpy(out_r, in_r, sizeof(Sample) * numSamples);
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
    }
};

class PassthroughPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Passthrough", {"Param1", "Param2"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<Passthrough>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<PassthroughPlugin>();
}

