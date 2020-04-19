#include "configuration_backend.h"

using namespace awesomefx;

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
}
