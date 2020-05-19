#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <random>

namespace awesomefx
{

class WhiteNoiseSource : public AudioProcessor
{
  public:
    WhiteNoiseSource(const AudioProcessingContext& context)
    {
      std::random_device rd;
      m_gen.seed(rd());
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto rnd = m_dis(m_gen) * m_volume;
        *out_l++ = rnd;
        *out_r++ = rnd;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_volume = param.value;
    }

    float m_volume{};
    std::mt19937 m_gen{};
    std::uniform_real_distribution<> m_dis{-1.0, 1.0};
};

class WhiteNoiseSourcePlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"White noise source", {"Volume"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<WhiteNoiseSource>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<WhiteNoiseSourcePlugin>();
}

