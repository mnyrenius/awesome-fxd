#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>

namespace awesomefx
{

  namespace
  {
    const int DelayBufferSize = 4096;
    const int Delay1 = 0.25 * DelayBufferSize * 0.66;
    const int Delay2 = 0.25 * DelayBufferSize * 0.33;

    const int Rate = 0;
    const int Depth = 1;
    const int DryWet = 2;

    class DelayLine
    {
      public:
        DelayLine(std::size_t size)
        {
          m_buffer.resize(size);
          std::fill(m_buffer.begin(), m_buffer.end(), 0);
        }

        void write(Sample s)
        {
          m_buffer[m_w++] = s;
          if (m_w >= m_buffer.size()) m_w = 0;
        }

        Sample read(float delay)
        {
          auto r = m_w - delay;
          if (r < 0) r += m_buffer.size();
          int i = r;
          float f = r - i;
          auto s1 = m_buffer[i];
          if (++i >= m_buffer.size()) i = 0;
          return s1 + (m_buffer[i] - s1) * f;
        }

      private:
        std::vector<Sample> m_buffer;
        std::uint32_t m_w = 0;
    };
  }

  class Chorus : public AudioProcessor
  {
    public:
      Chorus(const AudioProcessingContext& context)
        : m_fs(context.getSampleRate())
      {
      }

      void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
      {
        while (numSamples--)
        {
          auto left = *in_l++;
          auto right = *in_r++;

          m_delayLine1_l.write(left);
          m_delayLine2_l.write(left);

          m_delayLine1_r.write(right);
          m_delayLine2_r.write(right);

          auto mod_l = 0.25 * DelayBufferSize * std::sin(2.0 * 3.141592 * m_phase) * m_depth;
          auto mod_r = 0.25 * DelayBufferSize * std::sin(2.0 * 3.141592 * m_phase * 0.8) * m_depth;

          auto wet_l = 0.5 * m_delayLine1_l.read(Delay1 + mod_l) +
                       0.5 * m_delayLine2_l.read(Delay2 + mod_l);

          auto wet_r = 0.5 * m_delayLine1_r.read(Delay1 * 0.8 + mod_r) +
                       0.5 * m_delayLine2_r.read(Delay2 * 0.8 + mod_r);

          *out_l++ = m_drywet * wet_l + (1 - m_drywet) * left;
          *out_r++ = m_drywet * wet_r + (1 - m_drywet) * right;

          m_phase += m_step;
        }
      }

      void setParameter(const AudioProcessor::Parameter& param) override
      {
        switch (param.index)
        {
          case Rate:
            m_step = 10.0 * param.value / m_fs;
          case Depth:
            m_depth = param.value * 0.15;
          case DryWet:
            m_drywet = param.value;
        }
      }

      DelayLine m_delayLine1_l{DelayBufferSize};
      DelayLine m_delayLine2_l{DelayBufferSize};
      DelayLine m_delayLine1_r{DelayBufferSize};
      DelayLine m_delayLine2_r{DelayBufferSize};
      float m_phase = 0;
      std::uint32_t m_fs;
      float m_step = 0;
      float m_depth = 0;
      float m_drywet = 0;
  };

  class ChorusPlugin : public FxPlugin
  {
    public:
      FxPluginInfo getPluginInfo() const override
      {
        return {"Chorus", {"Rate", "Depth", "Dry/Wet"}};
      }

      AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
      {
        return std::make_unique<Chorus>(context);
      }
  };

}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<ChorusPlugin>();
}

