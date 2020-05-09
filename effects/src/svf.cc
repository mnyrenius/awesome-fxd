#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
  const int Cutoff = 0;
  const int Resonance = 1;
  const int Lp = 2;
  const int Bp = 3;
  const int Hp = 4;
}

class StateVariableFilter : public AudioProcessor
{
  public:
    StateVariableFilter(const AudioProcessingContext& context)
    {
      m_fs = context.getSampleRate();
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        m_l_l = m_f1 * m_b_l + m_l_l;
        m_h_l = *in_l++ - m_l_l - m_q1 * m_b_l;
        m_b_l = m_f1 * m_h_l + m_b_l;

        m_l_r = m_f1 * m_b_r + m_l_r;
        m_h_r = *in_r++ - m_l_r - m_q1 * m_b_r;
        m_b_r = m_f1 * m_h_r + m_b_r;

        *out_l++ = m_l_l * m_params[Lp] + m_b_l * m_params[Bp] + m_h_l * m_params[Hp];
        *out_r++ = m_l_r * m_params[Lp] + m_b_r * m_params[Bp] + m_h_r * m_params[Hp];
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      m_params[param.index] = param.value;
      switch (param.index)
      {
        case Cutoff:
          m_f1 = 2.0 * std::sin(3.141592 * 2000 * m_params[Cutoff] / m_fs);
          break;
        case Resonance:
          m_q1 = 1 - m_params[Resonance];
          break;
        default:
          break;
      }
    }

    std::vector<float> m_params {0.5, 0.0, 0.0, 0.0, 0.0};
    Sample m_l_l = 0;
    Sample m_b_l = 0;
    Sample m_h_l = 0;
    Sample m_l_r = 0;
    Sample m_b_r = 0;
    Sample m_h_r = 0;
    float m_f1 = 0;
    float m_q1 = 0;
    std::uint32_t m_fs;
};

class StateVariableFilterPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"State Variable Filter", {"Cutoff", "Resonance", "LP Mix", "BP Mix", "HP Mix"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<StateVariableFilter>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<StateVariableFilterPlugin>();
}
