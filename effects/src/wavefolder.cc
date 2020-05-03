#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace
{
  float sign(float x)
  {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
  }
}

namespace awesomefx
{

class WaveFolder : public AudioProcessor
{
  public:
    WaveFolder(const AudioProcessingContext& context)
    {
      m_params = {0.5};
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto x_l = m_params[0] * *in_l++;
        auto x_r = m_params[0] * *in_r++;
        *out_l++ = sign(x_l) * 2 * std::abs(x_l / 2 - std::round(x_l / 2));
        *out_r++ = sign(x_r) * 2 * std::abs(x_r / 2 - std::round(x_r / 2));
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value * 10;
    }

    std::vector<float> m_params;
};

class WaveFolderPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Wavefolder", {"Gain"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<WaveFolder>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<WaveFolderPlugin>();
}
