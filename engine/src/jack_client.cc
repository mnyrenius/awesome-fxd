#include <jack_client.h>
#include <stdexcept>
#include <cstdio>
#include <memory>

using namespace awesomefx;

namespace
{

const std::size_t RingBufferSize = 8192;

int process(jack_nframes_t nframes, void *arg)
{
  auto& data = *static_cast<JackClientImpl::ProcessCtx*>(arg);

  if (::jack_ringbuffer_read_space(data.ringBuffer) >= sizeof(AudioProcessor::Parameter))
  {
    AudioProcessor::Parameter parameter;
    auto size = ::jack_ringbuffer_read(
        data.ringBuffer,
        reinterpret_cast<char *>(&parameter),
        sizeof(parameter));

    if (size == sizeof(parameter))
    {
      data.processor->setParameter(parameter);
    }
  }

  data.processor->process(
      static_cast<Sample *>(::jack_port_get_buffer(data.inputPorts.left, nframes)),
      static_cast<Sample *>(::jack_port_get_buffer(data.inputPorts.right, nframes)),
      static_cast<Sample *>(::jack_port_get_buffer(data.outputPorts.left, nframes)),
      static_cast<Sample *>(::jack_port_get_buffer(data.outputPorts.right, nframes)),
      nframes);

  return 0;
}
}

JackClientImpl::JackClientImpl(const std::string& name, const AudioProcessor::Factory& processorFactory)
{
  jack_status_t status;
  m_client = ::jack_client_open(name.c_str(), JackNullOption, &status, 0);

  if (!m_client)
  {
    throw std::runtime_error("jack_client_open failed");
  }

  m_processCtx.ringBuffer = ::jack_ringbuffer_create(RingBufferSize);
  m_processCtx.processor = processorFactory(*this);

  ::jack_set_process_callback(m_client, process, &m_processCtx);

  auto in_left = ::jack_port_register (m_client, "in_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  auto in_right = ::jack_port_register (m_client, "in_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  auto out_left = ::jack_port_register (m_client, "out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  auto out_right = ::jack_port_register (m_client, "out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

  if (!in_left || !in_right || !out_left || !out_right)
  {
    throw std::runtime_error("Failed to create ports");
  }

  m_processCtx.inputPorts = { in_left, in_right };
  m_processCtx.outputPorts = { out_left, out_right };

  if (::jack_activate(m_client))
  {
    throw std::runtime_error("Failed to activate client");
  }
}

JackClientImpl::~JackClientImpl()
{
  ::jack_client_close(m_client);
}

void JackClientImpl::connectInputsToCapturePorts(std::vector<std::string> portNames) const
{

  if (portNames.size() != 2)
  {
    auto ports = ::jack_get_ports (m_client, 0, 0, JackPortIsPhysical|JackPortIsOutput);
    if (ports == nullptr)
    {
      throw std::runtime_error("No physical capture ports");
    }

    for (auto i = 0U; ports[i]; ++i)
    {
      portNames.push_back(ports[i]);
    }

    ::jack_free(ports);
  }

  if (portNames.size() < 2)
  {
    throw std::runtime_error("There must be at least 2 capture ports");
  }

  auto res1 = ::jack_connect(
      m_client,
      portNames[0].c_str(),
      ::jack_port_name(m_processCtx.inputPorts.left));

  auto res2 = ::jack_connect(
      m_client,
      portNames[1].c_str(),
      ::jack_port_name(m_processCtx.inputPorts.right));

  if (res1 || res2)
  {
    throw std::runtime_error("Failed to connect input to capture ports");
  }
}

std::vector<std::string> JackClientImpl::getInputPorts() const
{
  auto& ports = m_processCtx.inputPorts;
  return { ::jack_port_name(ports.left), ::jack_port_name(ports.right) };
}

std::vector<std::string> JackClientImpl::getOutputPorts() const
{
  auto& ports = m_processCtx.outputPorts;
  return { ::jack_port_name(ports.left), ::jack_port_name(ports.right) };
}

void JackClientImpl::connectInputs(const std::vector<std::string>& portNames) const
{
  auto res1 = ::jack_connect(
      m_client,
      portNames[0].c_str(),
      ::jack_port_name(m_processCtx.inputPorts.left));

  auto res2 = ::jack_connect(
      m_client,
      portNames[1].c_str(),
      ::jack_port_name(m_processCtx.inputPorts.right));

  printf("Connected ins %s and %s to %s and %s\n",
      ::jack_port_name(m_processCtx.inputPorts.left),
      ::jack_port_name(m_processCtx.inputPorts.right),
      portNames[0].c_str(),
      portNames[1].c_str()
      );


  if (res1 || res2)
  {
    throw std::runtime_error("Failed to connect input ports");
  }
}

void JackClientImpl::connectOutputs(const std::vector<std::string>& portNames) const
{
  auto res1 = ::jack_connect(
      m_client,
      ::jack_port_name(m_processCtx.outputPorts.left),
      portNames[0].c_str());

  auto res2 = ::jack_connect(
      m_client,
      ::jack_port_name(m_processCtx.outputPorts.right),
      portNames[1].c_str());

  printf("Connected outs %s and %s to %s and %s\n",
      ::jack_port_name(m_processCtx.outputPorts.left),
      ::jack_port_name(m_processCtx.outputPorts.right),
      portNames[0].c_str(),
      portNames[1].c_str()
      );

  if (res1 || res2)
  {
    throw std::runtime_error("Failed to connect output ports");
  }
}

void JackClientImpl::connectOutputsToPlaybackPorts() const
{
  auto ports = ::jack_get_ports (m_client, 0, 0, JackPortIsPhysical|JackPortIsInput);
  if (ports == nullptr)
  {
    throw std::runtime_error("No physical playback ports");
  }

  std::vector<std::string> portNames;
  for (auto i = 0U; ports[i]; ++i)
  {
    portNames.push_back(ports[i]);
  }

  ::jack_free(ports);

  if (portNames.size() < 2)
  {
    throw std::runtime_error("There must be at least 2 physical playback ports");
  }

  auto res1 = ::jack_connect(
      m_client,
      ::jack_port_name(m_processCtx.outputPorts.left),
      portNames[0].c_str());

  auto res2 = ::jack_connect(
      m_client,
      ::jack_port_name(m_processCtx.outputPorts.right),
      portNames[1].c_str());

  printf("Connected %s and %s\n",
      ::jack_port_name(m_processCtx.outputPorts.left),
      ::jack_port_name(m_processCtx.outputPorts.right)
      );
  if (res1 || res2)
  {
    throw std::runtime_error("Failed to connect output ports to playback ports");
  }
}

void JackClientImpl::setParameter(const AudioProcessor::Parameter& parameter) const
{
  auto size = ::jack_ringbuffer_write(
      m_processCtx.ringBuffer,
      reinterpret_cast<const char *>(&parameter),
      sizeof(parameter));

  if (size != sizeof(parameter))
  {
    throw std::runtime_error("Failed to write parameter to ringbuffer");
  }
}

std::uint32_t JackClientImpl::getSampleRate() const
{
  return ::jack_get_sample_rate(m_client);
}
