#include <audio_processor.h>
#include <fx_plugin.h>
#include <memory>
#include <cmath>
#include <tuple>

namespace awesomefx
{

namespace
{
const std::uint32_t PredelayTime = 0;
const std::uint32_t InputBandwidth = 1;
const std::uint32_t Decay = 2;
const std::uint32_t Damping = 3;
const std::uint32_t ModRate = 4;
const std::uint32_t ModDepth = 5;
const std::uint32_t DryWet = 6;

class Allpass
{
  public:
    Allpass(std::uint32_t size, float fb_gain, float ff_gain)
      : size_(size)
      , fb_gain_(fb_gain)
      , ff_gain_(ff_gain)
    {
      buffer_.resize(size);
      std::fill(buffer_.begin(), buffer_.end(), 0);
    }

    inline Sample process(Sample in, float delay)
    {
      auto m = index_ - delay;
      if (m < 0) m += size_;

      const auto y = buffer_[m] + ff_gain_ * in;
      buffer_[index_++] = in + fb_gain_ * y;
      if (index_ >= size_) index_ = 0;

      return y;
    }

    inline Sample tap(std::uint32_t index) const
    {
      std::int32_t m = index_ - index;
      if (m < 0) m += size_;
      return buffer_[m];
    }

    inline std::uint32_t size() const
    {
      return size_;
    }

  private:
    std::vector<Sample> buffer_{};
    std::uint32_t index_{};
    std::uint32_t size_{};
    float fb_gain_{};
    float ff_gain_{};
};

class LPFilter
{
  public:
    inline void setGain(float gain, float fb_gain)
    {
      gain_ = gain;
      fb_gain_ = fb_gain;
    }

    inline Sample process(Sample input)
    {
      return x1_ = gain_ * input + fb_gain_ * x1_;
    }

  private:
    Sample x1_{};
    float gain_{};
    float fb_gain_{};
};

class Delay
{
  public:
    Delay(std::uint32_t size)
      : size_(size)
    {
      buffer_.resize(size);
      std::fill(buffer_.begin(), buffer_.end(), 0);
    }

    inline Sample read(float delay) const
    {
      auto m = index_ - delay;
      if (m < 0) m += size_;

      return buffer_[m];
    }

    inline void write(Sample in)
    {
      buffer_[index_++] = in;
      if (index_ >= size_) index_ = 0;
    }

    inline std::uint32_t size() const
    {
      return size_;
    }

  private:
    std::vector<Sample> buffer_;
    std::uint32_t size_{};
    std::uint32_t index_{};
};

class ReverbTank
{
  public:
    ReverbTank(std::uint32_t fs)
      : fs_(fs)
    {
    }

    inline void setDecay(float decay)
    {
      decay_ = decay;
    }

    inline void setDamping(float damping)
    {
      damping_left_.setGain(1 - damping, damping);
      damping_right_.setGain(1 - damping, damping);
    }

    inline void setModRate(float rate)
    {
      modStep_ = 3 * rate;
    }

    inline void setModDepth(float depth)
    {
      modDepth_ = depth;
    }

    inline std::tuple<Sample, Sample> process(Sample input)
    {
      Sample out_l = 0.0;
      Sample out_r = 0.0;

      const auto mod = std::sin(2 * 3.141592 * modPhase_ / fs_) * 128.0 * modDepth_;
      modPhase_ += modStep_;

      auto tank1 = decay_diffusion_1_left_.process(input, Diffusion1BaseDelayLeft - 1.0 + mod) + decay_ * delay_2_right_.read(delay_2_right_.size() - 1);
      delay_1_left_.write(tank1);
      tank1 = delay_1_left_.read(delay_1_left_.size() - 1);
      tank1 = damping_left_.process(tank1);
      tank1 = decay_diffusion_2_left_.process(tank1 * decay_, decay_diffusion_2_left_.size() - 1);
      delay_2_left_.write(tank1);

      auto tank2 = decay_diffusion_1_right_.process(input, Diffusion1BaseDelayRight - 1.0 + mod) + decay_ * delay_2_left_.read(delay_2_left_.size() - 1);
      delay_1_right_.write(tank2);
      tank2 = delay_1_right_.read(delay_1_right_.size() - 1);
      tank2 = damping_right_.process(tank2);
      tank2 = decay_diffusion_2_right_.process(tank2 * decay_, decay_diffusion_2_right_.size() - 1);
      delay_2_right_.write(tank2);

      const float ratio = fs_ / 29761.0;
      out_l += 0.6 * delay_1_right_.read(266 * ratio);
      out_l += 0.6 * delay_1_right_.read(2974 * ratio);
      out_l -= 0.6 * decay_diffusion_2_right_.tap(1913 * ratio);
      out_l += 0.6 * delay_2_right_.read(1996 * ratio);
      out_l -= 0.6 * delay_1_left_.read(1990 * ratio);
      out_l -= 0.6 * decay_diffusion_2_left_.tap(187 * ratio);
      out_l -= 0.6 * delay_2_left_.read(1066 * ratio);

      out_r += 0.6 * delay_1_left_.read(353 * ratio);
      out_r += 0.6 * delay_1_left_.read(3627 * ratio);
      out_r -= 0.6 * decay_diffusion_2_left_.tap(1228 * ratio);
      out_r += 0.6 * delay_2_left_.read(2673 * ratio);
      out_r -= 0.6 * delay_2_right_.read(2111 * ratio);
      out_r -= 0.6 * decay_diffusion_2_right_.tap(335 * ratio);
      out_r -= 0.6 * delay_2_right_.read(121 * ratio);

      return {out_l, out_r};
    }

  private:
    // left side of tank
    const std::uint32_t Diffusion1BaseDelayLeft = 995;
    Allpass decay_diffusion_1_left_{ Diffusion1BaseDelayLeft + 128, 0.7, -0.7 };
    Allpass decay_diffusion_2_left_{ 2667, -0.5, 0.5 };
    Delay delay_1_left_{ 6598 };
    LPFilter damping_left_{};
    Delay delay_2_left_{ 5512 };

    // right side of tank
    const std::uint32_t Diffusion1BaseDelayRight = 1345;
    Allpass decay_diffusion_1_right_{ Diffusion1BaseDelayRight + 128, 0.7, -0.7 };
    Allpass decay_diffusion_2_right_{ 3935, -0.5, 0.5 };
    Delay delay_1_right_{ 6248 };
    LPFilter damping_right_{};
    Delay delay_2_right_{ 4687 };

    float decay_{};
    std::uint32_t fs_;
    float modPhase_{};
    float modStep_{};
    float modDepth_{};
};

}

  class Reverb2 : public AudioProcessor
  {
    public:
      Reverb2(const AudioProcessingContext& context)
        : fs_(context.getSampleRate())
      {
        predelay_time_ = 5000;
        predelay_filter_.setGain(0.9995, 1 - 0.9995);
        reverbTank_.setDecay(0.5);
        reverbTank_.setDamping(0.0005);
      }

      void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) override
      {
        // Plate-class reverb from J. Dattorro, Effect Design Part 1: Reverberator and Other Filters
        while (numSamples--)
        {
          const auto left = *in_l++;
          const auto right = *in_r++;

          const auto in = 0.5 * (left + right);

          // Predelay + low pass filter
          predelay_.write(in);
          auto predelayed = predelay_.read(predelay_time_);
          predelayed = predelay_filter_.process(predelayed);

          // Input Diffusers
          auto diffused = predelayed;
          for (auto& ap : input_diffusion_aps_)
          {
            diffused = ap.process(diffused, ap.size() - 1);
          }

          // Tank
          const auto wet = reverbTank_.process(diffused);

          *out_l++ = std::get<0>(wet) * dryWet_ + left * (1 - dryWet_);
          *out_r++ = std::get<1>(wet) * dryWet_ + right * (1 - dryWet_);
        }
      }

      void setParameter(const AudioProcessor::Parameter& param) override
      {
        switch (param.index)
        {
          case PredelayTime:
            predelay_time_ = 20000 * param.value;
            break;
          case InputBandwidth:
            predelay_filter_.setGain(param.value, 1 - param.value);
            break;
          case Decay:
            reverbTank_.setDecay(param.value);
            break;
          case Damping:
            reverbTank_.setDamping(param.value);
            break;
          case ModRate:
            reverbTank_.setModRate(param.value);
            break;
          case ModDepth:
            reverbTank_.setModDepth(param.value);
            break;
          case DryWet:
            dryWet_ = param.value;
            break;
          default:
            break;
        }
      }

      std::uint32_t fs_;
      float dryWet_{};
      float predelay_time_{};
      Delay predelay_{20001};
      LPFilter predelay_filter_{};
      std::vector<Allpass> input_diffusion_aps_{ { 210, -0.75, 0.75 }, { 148, -0.75, 0.75  }, { 561, -0.625, 0.625  }, { 410, -0.625, 0.625  } };
      ReverbTank reverbTank_{fs_};
  };

  class Reverb2Plugin : public FxPlugin
  {
    public:
      FxPluginInfo getPluginInfo() const override
      {
        return {"Reverb 2", {"Predelay", "Input bandwidth", "Decay", "Damping", "Mod rate", "Mod depth", "Dry/Wet"}};
      }

      AudioProcessor::Ptr createAudioProcessor(const AudioProcessingContext& context) const override
      {
        return std::make_unique<Reverb2>(context);
      }
  };
}

using namespace awesomefx;

extern "C" FxPlugin::Ptr createFxPlugin()
{
  return std::make_unique<Reverb2Plugin>();
}

