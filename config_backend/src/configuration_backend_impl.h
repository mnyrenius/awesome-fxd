#ifndef CONFIGURATION_BACKEND_IMPL_H
#define CONFIGURATION_BACKEND_IMPL_H

#include <configuration_backend.h>
#include <boost/asio/io_context.hpp>
#include <http/reactor/listener.hxx>
#include <http/reactor/session.hxx>
#include <http/basic_router.hxx>
#include <http/out.hxx>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>

using namespace _0xdead4ead;
namespace beast = boost::beast;

using http_session = http::reactor::_default::session_type;
using http_listener = http::reactor::_default::listener_type;
using http_session = http::reactor::_default::session_type;
using http_listener = http::reactor::_default::listener_type;

using http_context = typename http_session::context_type;
using beast_http_request = typename http_session::request_type;
using asio_socket = typename http_session::socket_type;

namespace awesomefx
{

class ConfigurationBackendImpl : public ConfigurationBackend
{
  public:
    ConfigurationBackendImpl(boost::asio::io_context& io);
    ~ConfigurationBackendImpl() = default;
    void registerOnGetPlugins(const OnGetPluginsCallback& callback) override;
    void registerOnApplyConfig(const OnApplyConfigCallback& callback) override;
    void registerOnGetConfig(const OnGetConfigCallback& callback) override;
    void registerOnSetParameters(const OnSetParametersCallback& callback) override;
    void registerOnReload(const OnReloadCallback& callback) override;
    void start(std::uint32_t port) override;

  private:
    OnGetPluginsCallback m_getPlugins;
    OnApplyConfigCallback m_applyConfig;
    OnGetConfigCallback m_getConfig;
    OnSetParametersCallback m_setParameters;
    OnReloadCallback m_reload;
    boost::asio::io_context& m_io;
    std::unique_ptr<http::basic_router<http_session>> m_router =
      std::make_unique<http::basic_router<http_session>>(std::regex::ECMAScript);
};

}

#endif /* CONFIGURATION_BACKEND_IMPL_H */
