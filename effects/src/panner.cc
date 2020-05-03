#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace
{
  const int Sweep = 0;
  const int Depth = 1;
}

namespace awesomefx
{

class SimplePanner : public AudioProcessor
{
  public:
    SimplePanner(const AudioProcessingContext& context)
    {
      m_fs = context.getSampleRate();
      m_t = 0;
      m_params = {0.5, 0.0};
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto pan = 0.5 + m_params[Depth] * 0.5 * std::sin(2 * 3.141592 * m_params[Sweep] * m_t);
        m_t += 10.0 / m_fs;
        *out_l++ = (1 - pan) * *in_l++;
        *out_r++ = pan * *in_r++;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value;
    }

    std::uint32_t m_fs;
    float m_t;
    std::vector<float> m_params;
};

class SimplePannerPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Simple Panner", {"Sweep Frequency", "Depth"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<SimplePanner>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<SimplePannerPlugin>();
}
