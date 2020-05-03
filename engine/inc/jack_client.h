#ifndef JACK_CLIENT_H
#define JACK_CLIENT_H

#include <string>
#include <vector>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <audio_processor.h>

namespace awesomefx
{

class JackClient
{
  public:
    using Ptr = std::unique_ptr<JackClient>;
    using Factory = std::function<Ptr(const std::string&, AudioProcessor::Factory)>;

    virtual ~JackClient() {}
    virtual void connectInputsToCapturePorts(std::vector<std::string> portNames) const = 0;
    virtual void connectOutputsToPlaybackPorts() const = 0;
    virtual std::vector<std::string> getInputPorts() const = 0;
    virtual std::vector<std::string> getOutputPorts() const = 0;
    virtual void connectInputs(const std::vector<std::string>& portNames) const = 0;
    virtual void connectOutputs(const std::vector<std::string>& portNames) const = 0;
    virtual void setParameter(const AudioProcessor::Parameter& parameter) const = 0;
};

class JackClientImpl : public JackClient,
                       public AudioProcessingContext
{
  public:
    struct PortPair
    {
      jack_port_t* left;
      jack_port_t* right;
    };

    struct ProcessCtx
    {
      PortPair inputPorts;
      PortPair outputPorts;
      AudioProcessor::Ptr processor;
      jack_ringbuffer_t *ringBuffer;
    };

    JackClientImpl(const std::string& name, const AudioProcessor::Factory& processorFactory);
    ~JackClientImpl() override;
    JackClientImpl() = delete;

    void connectInputsToCapturePorts(std::vector<std::string> portNames) const override;
    void connectOutputsToPlaybackPorts() const override;
    std::vector<std::string> getInputPorts() const override;
    std::vector<std::string> getOutputPorts() const override;
    void connectInputs(const std::vector<std::string>& portNames) const override;
    void connectOutputs(const std::vector<std::string>& portNames) const override;
    void setParameter(const AudioProcessor::Parameter& parameter) const override;

    std::uint32_t getSampleRate() const override;

  private:

    jack_client_t* m_client;
    ProcessCtx m_processCtx;
};

}
#endif /* JACK_CLIENT_H */
