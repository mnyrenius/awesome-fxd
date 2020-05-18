#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

namespace
{
const int Size = 0;
const int DryWet = 1;

const int AllpassBufferSize = 2048 * 2;
const int CombBufferSize = 16384 * 2;

class Allpass
{
  public:
    Allpass(float gain)
      : m_gain(gain)
    {
      m_buffer.resize(AllpassBufferSize);
    }

    inline Sample filter(Sample in, float delay)
    {
      auto m = m_index - delay;
      if (m < 0) m += AllpassBufferSize;

      auto y = m_buffer[m] - m_gain * in;
      m_buffer[m_index++] = in + y * m_gain;
      if (m_index >= AllpassBufferSize) m_index = 0;

      return y;
    }

  private:
    std::vector<Sample> m_buffer;
    int m_index = 0;
    float m_gain;
};

class FbComb
{
  public:
    FbComb(float gain)
      : m_gain(gain)
    {
      m_buffer.resize(CombBufferSize);
    }

    inline Sample filter(Sample in, float delay)
    {
      auto m = m_index - delay;
      if (m < 0) m += CombBufferSize;

      auto y = m_buffer[m];

      auto out = m_buffer[m_index++] = in + m_gain * y;
      if (m_index >= CombBufferSize) m_index = 0;

      return out;
    }

  private:
    std::vector<Sample> m_buffer;
    int m_index = 0;
    float m_gain;
};

}

  class Reverb1 : public AudioProcessor
  {
    public:
      Reverb1(const AudioProcessingContext& context)
        : m_fs(context.getSampleRate())
      {
      }

      void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
      {
        // Schroeder reverb with 4 parallell comb filters + 3 allpass filters in series
        while (numSamples--)
        {
          Sample y_l = 0, y_r = 0;
          auto left = *in_l++;
          auto right = *in_r++;

          for (auto i = 0U; i < m_lFbcf.size(); ++i)
          {
            y_l += 0.25 * m_lFbcf[i].filter(left, m_combDelays[i] * m_size);
            y_r += 0.25 * m_rFbcf[i].filter(right, m_combDelays[i] * m_size);
          }

          for (auto i = 0U; i < m_lAps.size(); ++i)
          {
            y_l = m_lAps[i].filter(y_l, m_apDelays[i] * m_size);
            y_r = m_rAps[i].filter(y_r, m_apDelays[i] * m_size);
          }

          *out_l++ = y_l * m_dryWet + left * (1 - m_dryWet);
          *out_r++ = y_r * m_dryWet + right * (1 - m_dryWet);
        }
      }

      void setParameter(const AudioProcessor::Parameter& param) override
      {
        switch (param.index)
        {
          case Size:
            m_size = param.value;
            break;
          case DryWet:
            m_dryWet = param.value;
            break;
          default:
            break;
        }
      }

      std::uint32_t m_fs;
      std::vector<Allpass> m_lAps { { 0.7 }, { 0.7}, { 0.7 } };
      std::vector<Allpass> m_rAps { { 0.7 }, { 0.7}, { 0.7 } };
      std::vector<float> m_apDelays { 1853.964 * 2, 594.468 * 2, 199.332 * 2 };
      std::vector<FbComb> m_lFbcf { { 0.742 }, { 0.733 }, { 0.715 }, { 0.697 } };
      std::vector<FbComb> m_rFbcf { { 0.742 }, { 0.733 }, { 0.715 }, { 0.697 } };
      std::vector<float> m_combDelays { 8465.436 * 2, 8818.236 * 2, 9523.836 * 2, 10232.964 * 2 };
      float m_size = 0.0;
      float m_dryWet = 0.0;
  };

  class Reverb1Plugin : public FxPlugin
  {
    public:
      FxPluginInfo getPluginInfo() const override
      {
        return {"Reverb 1", {"Size", "Dry/Wet"}};
      }

      AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
      {
        return std::make_unique<Reverb1>(context);
      }
  };

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<Reverb1Plugin>();
}

