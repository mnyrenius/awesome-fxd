#include "configuration_backend_impl.h"
#include <nlohmann/json.hpp>
#include <algorithm>

using namespace awesomefx;
using json = nlohmann::json;

namespace
{

template<class ResponseBody, class RequestBody>
beast::http::response<ResponseBody>
make_200(const beast::http::request<RequestBody>& request,
         typename ResponseBody::value_type body,
         beast::string_view content)
{
    beast::http::response<ResponseBody> response{beast::http::status::ok, request.version()};
    response.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(beast::http::field::content_type, content);
    response.set(beast::http::field::access_control_allow_origin, "*");
    response.body() = body;
    response.prepare_payload();
    response.keep_alive(request.keep_alive());

    return response;
}

template<class ResponseBody, class RequestBody>
beast::http::response<ResponseBody>
make_404(const beast::http::request<RequestBody>& request,
         typename ResponseBody::value_type body,
         beast::string_view content)
{
    beast::http::response<ResponseBody> response{beast::http::status::not_found, request.version()};
    response.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(beast::http::field::content_type, content);
    response.body() = body;
    response.prepare_payload();
    response.keep_alive(request.keep_alive());

    return response;
}
}

ConfigurationBackendImpl::ConfigurationBackendImpl(boost::asio::io_context& io)
  : m_io(io)
{
}

void ConfigurationBackendImpl::registerOnGetPlugins(const OnGetPluginsCallback& callback)
{
  m_getPlugins = callback;
}

void ConfigurationBackendImpl::registerOnApplyConfig(const OnApplyConfigCallback& callback)
{
  m_applyConfig = callback;
}

void ConfigurationBackendImpl::registerOnGetConfig(const OnGetConfigCallback& callback)
{
  m_getConfig = callback;
}

void ConfigurationBackendImpl::start(std::uint32_t port)
{
  m_router->get(R"(^/plugins$)", [this](beast_http_request r, http_context c) {
      json reply;
      for (auto& plugin : m_getPlugins())
      {
        reply.push_back({{"name", plugin.name}, {"parameters", plugin.parameters}});
      }

      c.send(make_200<beast::http::string_body>(r, reply.dump(), "application/json"));
      });

  m_router->get(R"(^/config$)", [this](beast_http_request r, http_context c) {
      json reply;
      for (auto& plugin : m_getConfig())
      {
        reply.push_back({{"name", plugin.name}, {"parameters", plugin.parameters}});
      }

      c.send(make_200<beast::http::string_body>(r, reply.dump(), "application/json"));
      });

  m_router->options(R"(^/config$)", [this](beast_http_request r, http_context c) {
      beast::http::response<beast::http::string_body> response{beast::http::status::ok, r.version()};
      response.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      response.set(beast::http::field::access_control_allow_origin, "*");
      response.set(beast::http::field::access_control_allow_methods, "GET, POST, PUT");
      response.set(beast::http::field::access_control_allow_headers, "Content-Type");
      response.prepare_payload();
      response.keep_alive(r.keep_alive());
      c.send(response);
      });

  m_router->put(R"(^/config$)", [this](beast_http_request r, http_context c) {
      auto json = json::parse(r.body());
      FxChainConfiguration config;
      std::transform(
          json.begin(),
          json.end(),
          std::back_inserter(config),
          [](auto& fx) {
            return FxConfiguration {fx["name"], fx["parameters"]};
          });

      m_applyConfig(config);

      c.send(make_200<beast::http::string_body>(r, json.dump(), "application/json"));
      });

  m_router->all(R"(^.*$)", [](beast_http_request r, http_context c) {
      c.send(make_404<beast::http::string_body>(r, "not found", "text/html"));
      });

    auto onError = [this](boost::system::error_code code, boost::string_view from) {
        printf("Error: %s", code.message().c_str());

        if (code == boost::system::errc::address_in_use or
                code == boost::system::errc::permission_denied)
            m_io.stop();
    };

    auto onAccept = [&](asio_socket s) {
        auto endpoint = s.remote_endpoint();
        http_session::recv(std::move(s), *m_router, onError);
    };

    auto address = boost::asio::ip::address_v4::any();

    printf("Starting configuration backend on %s:%u\n", address.to_string().c_str(), port);
    http_listener::launch(m_io, {address, static_cast<uint16_t>(port)}, onAccept, onError);
}
