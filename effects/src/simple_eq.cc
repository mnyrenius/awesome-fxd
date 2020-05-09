#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

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

template<int Sign>
class ShelvingFilter
{
  public:
    inline void calcCoeffs(std::uint32_t fs, float cutoff)
    {
      m_c_b = (std::tan(3.141592 * cutoff / fs) - 1) / (std::tan(3.141592 * cutoff / fs) + 1);
    }

    inline void setGain(float gain)
    {
      m_v0 = std::pow(10.0, (gain * 24) / 20.0);
    }

    inline Sample filter(Sample in)
    {
      auto xh_new = in - m_c_b * m_x_h;
      auto y_l = m_c_b * xh_new + m_x_h;
      m_x_h = xh_new;
      return 0.5 * (m_v0 - 1) * (in + y_l * Sign) + in;
    }

  private:
    float m_c_b = 0;
    float m_v0 = 0;
    float m_x_h = 0;
};

class PeakFilter
{
  public:
    inline void calcCoeffs(std::uint32_t fs, float cutoff, float bandwidth)
    {
      m_d = -std::cos(2.0 * 3.141592 * cutoff / fs);
      m_c_b = (std::tan(3.141592 * bandwidth / fs) - 1) / (std::tan(3.141592 * bandwidth / fs) + 1);
    }

    inline void setGain(float gain)
    {
      m_v0 = std::pow(10, gain * 32.0 / 20.0);
    }

    inline Sample filter(Sample in)
    {
      auto xh = in - m_d * (1 - m_c_b) * m_x_h_1 + m_c_b * m_x_h_2;
      auto y_l = -m_c_b * xh + m_d * (1 - m_c_b) * m_x_h_1 + m_x_h_2;
      m_x_h_1 = xh;
      m_x_h_2 = m_x_h_1;
      return 0.5 * (m_v0 - 1) * (in - y_l) + in;
    }

  private:
    float m_x_h_1 = 0;
    float m_x_h_2 = 0;
    float m_v0 = 0;
    float m_d = 0;
    float m_c_b = 0;
};
}

class SimpleEq : public AudioProcessor
{
  public:
    SimpleEq(const AudioProcessingContext& context)
      : m_fs(context.getSampleRate())
    {
      m_low_l.calcCoeffs(m_fs, CutoffLow);
      m_low_r.calcCoeffs(m_fs, CutoffLow);
      m_high_l.calcCoeffs(m_fs, CutoffHigh);
      m_high_r.calcCoeffs(m_fs, CutoffHigh);
      m_peak_l.calcCoeffs(m_fs, CutoffMid, BandwidthMid);
      m_peak_r.calcCoeffs(m_fs, CutoffMid, BandwidthMid);
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
    ShelvingFilter<1> m_low_l;
    ShelvingFilter<-1> m_high_l;
    ShelvingFilter<1> m_low_r;
    ShelvingFilter<-1> m_high_r;
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

