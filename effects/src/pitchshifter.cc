#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  const int DelayLineSize = 2048;
  const int Pitch = 0;

  inline Sample interpolate(float phase, const std::vector<Sample>& delayLine)
  {
    int i = phase;
    float f = phase - i;
    auto first = delayLine[i];
    if (++i >= DelayLineSize) i = 0;
    auto second = delayLine[i];
    return first + (second - first) * f;
  }
}

class PitchShifter : public AudioProcessor
{
  public:
    PitchShifter(const AudioProcessingContext& context)
    {
      m_lDelayLine.reserve(DelayLineSize);
      m_rDelayLine.reserve(DelayLineSize);
      std::fill(m_lDelayLine.begin(), m_lDelayLine.end(), 0);
      std::fill(m_rDelayLine.begin(), m_rDelayLine.end(), 0);
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto left = *in_l++;
        auto right = *in_r++;

        m_index++;
        if (m_index >= DelayLineSize) m_index = 0;
        m_lDelayLine[m_index] = left;
        m_rDelayLine[m_index] = right;

        auto phase = m_phase * DelayLineSize;
        auto read2 = phase + 0.5 * DelayLineSize;
        if (read2 >= DelayLineSize) read2 -= DelayLineSize;

        auto fade = 2.0 * (m_phase >= 0.5 ? 1.0 - m_phase : m_phase);

        *out_l++ = interpolate(phase, m_lDelayLine) * fade + interpolate(read2, m_lDelayLine) * (1 - fade);
        *out_r++ = interpolate(phase, m_rDelayLine) * fade + interpolate(read2, m_rDelayLine) * (1 - fade);

        m_phase += 2 * m_params[Pitch] / DelayLineSize;
        if (m_phase >= 1.0) m_phase -= 1.0;
        if (m_phase <= 0.0) m_phase += 1.0;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value;
    }

    std::vector<float> m_params {0.5};
    std::vector<Sample> m_lDelayLine;
    std::vector<Sample> m_rDelayLine;
    std::uint32_t m_index = 0;
    float m_phase = 0;
};

class PitchShifterPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Pitch Shifter", {"Pitch"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<PitchShifter>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<PitchShifterPlugin>();
}

