#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <cstddef>
#include <memory>
#include <functional>
#include <string>
#include <cstdint>

namespace awesomefx
{

using Sample = float;

class AudioProcessingContext
{
  public:
    virtual ~AudioProcessingContext() {}
    virtual std::uint32_t getSampleRate() const = 0;
};

class AudioProcessor
{
  public:
    using Ptr = std::unique_ptr<AudioProcessor>;
    using Factory = std::function<Ptr(const AudioProcessingContext&)>;

    struct Parameter
    {
      std::uint32_t index;
      float value;
    };

    virtual ~AudioProcessor() {}

    virtual void process(Sample* in_l, Sample* in_r, Sample* out_l, Sample* out_r, std::size_t numSamples) = 0;
    virtual void setParameter(const Parameter& param) = 0;

};

}
#endif /* AUDIO_PROCESSOR_H */
