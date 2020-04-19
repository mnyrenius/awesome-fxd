#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "jack_client.h"
#include <memory>
#include <boost/program_options.hpp>
#include <audio_processor.h>
#include <fx_plugin.h>
#include "fx_plugin_handler.h"
#include "controller.h"
#include "configuration_backend.h"

namespace po = boost::program_options;

using namespace awesomefx;

int main(int argc, char *argv[])
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("input-ports", po::value<std::vector<std::string>>()->multitoken(), "set jack input ports")
    ("plugin-dir", po::value<std::vector<std::string>>(), "set plugin directory")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc;
    return 0;
  }

  std::vector<std::string> inputs;
  std::string pluginDir{"effects"};

  if (vm.count("input-ports"))
  {
    inputs = vm["input-ports"].as<std::vector<std::string>>();
  }

  if (vm.count("plugin-dir"))
  {
    pluginDir = vm["plugin-dir"].as<std::string>();
  }

  boost::asio::io_context io_context;
  auto work = boost::asio::make_work_guard(io_context);

  auto controller = std::make_unique<ControllerImpl>(
      inputs,
      std::make_unique<FxPluginHandlerImpl>(pluginDir),
      [](auto& name, auto processor) {
        return std::make_unique<JackClientImpl>(name, std::move(processor));
      },
      std::make_unique<ConfigurationBackendImpl>()
      );

  io_context.run();
}
