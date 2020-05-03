#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  const int DelayBufferSize = 128000;
  const int Time = 0;
  const int Feedback = 1;
  const int DryWet = 2;
}

class SimpleDelay : public AudioProcessor
{
  public:
    SimpleDelay(const AudioProcessingContext& context)
    {
      m_params = {0.2, 0.3, 0.5};
      m_lBuffer.reserve(DelayBufferSize);
      m_rBuffer.reserve(DelayBufferSize);
      std::fill(m_lBuffer.begin(), m_lBuffer.end(), 0.0);
      std::fill(m_rBuffer.begin(), m_rBuffer.end(), 0.0);
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        if (m_index >= DelayBufferSize) m_index = 0;
        auto j = m_index - (m_params[Time] * m_maxDelayTime);
        if (j < 0) j += DelayBufferSize;

        auto dry_l = *in_l++;
        auto dry_r = *in_r++;
        m_lBuffer[m_index] = dry_l + (m_lBuffer[j] * m_params[Feedback]);
        m_rBuffer[m_index] = dry_r + (m_rBuffer[j] * m_params[Feedback]);
        auto wet_l = m_lBuffer[j] * m_params[Feedback];
        auto wet_r = m_rBuffer[j] * m_params[Feedback];
        *out_l++ = m_params[DryWet] * wet_l + (1 - m_params[DryWet]) * dry_l;
        *out_r++ = m_params[DryWet] * wet_r + (1 - m_params[DryWet]) * dry_r;
        m_index++;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value;
    }

    std::vector<float> m_params;
    std::vector<Sample> m_lBuffer;
    std::vector<Sample> m_rBuffer;
    float m_maxDelayTime = DelayBufferSize / 2;
    int m_index = 0;

};

class SimpleDelayPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"SimpleDelay", {"Time", "Feedback", "Dry/Wet"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<SimpleDelay>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<SimpleDelayPlugin>();
}

