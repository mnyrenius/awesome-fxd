#include "fx_plugin_handler.h"
#include <boost/filesystem.hpp>
#include <dlfcn.h>
#include <algorithm>

using namespace boost::filesystem;
using namespace awesomefx;

FxPluginHandlerImpl::FxPluginHandlerImpl(const std::string& dir)
{
  for (auto& file : directory_iterator(dir))
  {
    auto& path = file.path();

    if (path.extension() != ".so")
    {
      continue;
    }

    auto handle = dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (!handle)
    {
      printf("Error: Failed to open %s\n", path.c_str());
      continue;
    }

    m_handles.push_back(handle);

    FxPlugin::Ptr(*createFxPlugin)() =
      (FxPlugin::Ptr(*)())dlsym(handle, "createFxPlugin");

    if (!createFxPlugin)
    {
      printf("Error: %s is not a valid fx plugin\n", path.c_str());
      dlclose(handle);
      continue;
    }

    auto plugin = createFxPlugin();
    auto name = plugin->getPluginInfo().name;
    m_fxPlugins[name] = std::move(plugin);
  }
}

FxPluginHandlerImpl::~FxPluginHandlerImpl()
{
  m_fxPlugins.clear();
  for (auto handle : m_handles)
  {
    if (dlclose(handle))
    {
      printf("Failed to close solib\n");
    }
  }
}

const FxPlugin& FxPluginHandlerImpl::getPlugin(const std::string& name) const
{
  auto it = m_fxPlugins.find(name);
  if (it == m_fxPlugins.end())
  {
    printf("Failed to find plugin: %s\n", name.c_str());
    throw std::runtime_error("Failed to find plugin");
  }

  return *(it->second);
}

std::vector<const FxPlugin *> FxPluginHandlerImpl::getAllPlugins() const
{
  std::vector<const FxPlugin *> plugins;
  std::transform(
      m_fxPlugins.begin(),
      m_fxPlugins.end(),
      std::back_inserter(plugins),
      [](auto& kv) {
        return kv.second.get();
      });
  return plugins;
}

