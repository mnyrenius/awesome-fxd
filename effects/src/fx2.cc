#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  inline Sample distort(Sample in, auto x)
  {
    if (std::fabs(in) < 0.33)
    {
      return 2 * in;
    }
    
    else if (std::fabs(in) >= 0.33 * 2)
    {
      if (x > 0)
      {
        return (3-(2-in*3)*(2-in*3))/3;
      }
      else if (x < 0)
      {
        return -(3-(2+in*3)*(2+in*3))/3;
      }
    }

    else
    {
      if (x > 0) return 1.0;
      else if (x < 0) return -1.0;
    }

    return 0.0;
  }
}

class SimpleDistortion : public AudioProcessor
{
  public:
    SimpleDistortion()
    {
      m_params = {1.0f};
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

    std::vector<float> m_params;
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

