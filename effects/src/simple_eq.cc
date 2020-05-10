#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>
#include <type_traits>

namespace awesomefx
{

namespace
{
const int Low = 0;
const int Mid = 1;
const int High = 2;

const float CutoffLow = 100;
const float CutoffMid = 1000;
const float CutoffHigh = 8000;
const float BandwidthMid = 800;

struct LowPass{};
struct HighPass{};

template<typename TFilter>
class ShelvingFilter
{
  public:
    ShelvingFilter(std::uint32_t fs, float cutoff)
      : m_fs(fs),
        m_cutoff(cutoff)
    {
      calcCoeffs();
    }

    inline void calcCoeffs()
    {
      auto tan = std::tan(3.141592 * m_cutoff / m_fs);
      m_c_b = (tan - 1) / (tan + 1);
      if (std::is_same<TFilter, LowPass>::value)
      {
        m_c_c = (tan - m_v0) / (tan + m_v0);
      }
      else
      {
        m_c_c = m_v0 * (tan - 1) / m_v0 * (tan + 1);
      }
    }

    inline void setGain(float gain)
    {
      gain -= 0.5;
      m_v0 = std::pow(10.0, gain * 48 / 20.0);
      if (gain >= 0.0)
      {
        m_c = m_c_b;
      }
      else
      {
        calcCoeffs();
        m_c = m_c_c;
      }
    }

    inline Sample filter(Sample in)
    {
      auto xh_new = in - m_c * m_x_h;
      auto y_l = m_c * xh_new + m_x_h;
      m_x_h = xh_new;
      if (std::is_same<TFilter, LowPass>::value)
      {
        return 0.5 * (m_v0 - 1) * (in + y_l) + in;
      }
      else
      {
        return 0.5 * (m_v0 - 1) * (in - y_l) + in;
      }
    }

  private:
    float m_c_b{};
    float m_c_c{};
    float m_v0{};
    float m_x_h{};
    float m_c{};
    float m_fs{};
    float m_cutoff{};
};

class PeakFilter
{
  public:
    PeakFilter(std::uint32_t fs, float cutoff, float bandwidth)
      : m_fs(fs)
      , m_cutoff(cutoff)
      , m_bandwidth(bandwidth)
    {
      calcCoeffs();
    }

    inline void calcCoeffs()
    {
      auto tan = std::tan(3.141592 * m_bandwidth / m_fs) ;
      m_d = -std::cos(2.0 * 3.141592 * m_cutoff / m_fs);
      m_c_b = (tan - 1) / (tan + 1);
      m_c_c = (tan - m_v0) / (tan + m_v0);
    }

    inline void setGain(float gain)
    {
      gain -= 0.5;
      m_v0 = std::pow(10, gain * 48.0 / 20.0);
      if (gain >= 0.0)
      {
        m_c = m_c_b;
      }
      else
      {
        calcCoeffs();
        m_c = m_c_c;
      }
    }

    inline Sample filter(Sample in)
    {
      auto xh = in - m_d * (1 - m_c) * m_x_h_1 + m_c * m_x_h_2;
      auto y_l = -m_c * xh + m_d * (1 - m_c) * m_x_h_1 + m_x_h_2;
      m_x_h_1 = xh;
      m_x_h_2 = m_x_h_1;
      return 0.5 * (m_v0 - 1) * (in - y_l) + in;
    }

  private:
    float m_x_h_1{};
    float m_x_h_2{};
    float m_v0{};
    float m_d{};
    float m_c_b{};
    float m_c_c{};
    float m_c{};
    float m_fs{};
    float m_cutoff{};
    float m_bandwidth{};
};
}

class SimpleEq : public AudioProcessor
{
  public:
    SimpleEq(const AudioProcessingContext& context)
      : m_fs(context.getSampleRate())
      , m_low_l(m_fs, CutoffLow)
      , m_low_r(m_fs, CutoffLow)
      , m_high_l(m_fs, CutoffHigh)
      , m_high_r(m_fs, CutoffHigh)
      , m_peak_l(m_fs, CutoffMid, BandwidthMid)
      , m_peak_r(m_fs, CutoffMid, BandwidthMid)
    {
    }

    void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
    {
      while (numSamples--)
      {
        auto left = *in_l++;
        auto right = *in_r++;

        auto y_l = m_low_l.filter(left);
        y_l = m_high_l.filter(y_l);
        y_l = m_peak_l.filter(y_l);

        auto y_r = m_low_r.filter(right);
        y_r = m_high_r.filter(y_r);
        y_r = m_peak_r.filter(y_r);

        *out_l++ = y_l;
        *out_r++ = y_r;
      }
    }

    void setParameter(const AudioProcessor::Parameter& param) override
    {
      switch (param.index)
      {
        case Low:
          m_low_l.setGain(param.value);
          m_low_r.setGain(param.value);
          break;
        case Mid:
          m_peak_l.setGain(param.value);
          m_peak_r.setGain(param.value);
          break;
        case High:
          m_high_r.setGain(param.value);
          m_high_l.setGain(param.value);
          break;
      }
    }

  private:
    float m_fs;
    ShelvingFilter<LowPass> m_low_l;
    ShelvingFilter<HighPass> m_high_l;
    ShelvingFilter<LowPass> m_low_r;
    ShelvingFilter<HighPass> m_high_r;
    PeakFilter m_peak_l;
    PeakFilter m_peak_r;

};

class SimpleEqPlugin : public FxPlugin
{
  public:
    FxPluginInfo getPluginInfo() const override
    {
      return {"Simple Eq", {"Low", "Mid", "High"}};
    }

    AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
    {
      return std::make_unique<SimpleEq>(context);
    }
};

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<SimpleEqPlugin>();
}

