#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  inline auto distort(auto in, auto x)
  {
    if (in < 0.0)
    {
      return std::pow(in + 1.0, x) - 1.0;
    }
    else
    {
      return std::pow(in - 1.0, x) + 1.0;
    }
  }
}

class SimpleDistortion : public AudioProcessor
{
  public:
    SimpleDistortion()
    {
      m_params = {1};
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        *out_l++ = distort(*in_l++, m_params[0]);
        *out_r++ = distort(*in_r++, m_params[0]);
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value;
    }

    std::vector<int32_t> m_params;
};

class SimpleDistortionPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"SimpleDistortion", {"Krschh"}};
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

