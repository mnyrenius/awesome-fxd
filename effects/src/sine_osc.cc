#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  std::vector<float> generateCMajor()
  {
    float current = 16.35;
    int semis = 2;
    std::vector<float> notes{current};

    for (auto i = 1U; i < 7 * 10; ++i)
    {
      current = current * std::pow(2, semis/12.0);
      notes.push_back(current);

      if (i % 7 == 2 || i % 7 == 6)
      {
        semis = 1;
      }
      else
      {
        semis = 2;
      }
    }

    return notes;
  }
}

class SineOscillator : public AudioProcessor
{
  public:
    SineOscillator(const AudioProcessingContext& context)
     : m_fs(context.getSampleRate())
     , m_notes(generateCMajor())
    {
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto t = m_phase++ * m_notes[m_index] / m_fs;
        auto s = std::sin(2 * 3.141592 * t);
        *out_l++ = *out_r++ = s;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_index = 70.0 * param.value;
    }

    float m_phase = 0;
    std::vector<float> m_notes;
    std::uint32_t m_fs;
    std::uint32_t m_index;
};

class SineOscillatorPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Sine Source (quantized)", {"Freq"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<SineOscillator>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<SineOscillatorPlugin>();
}
