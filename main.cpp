#include "pre.h"
#include "CommandConnection.h"
#include "AVRController.h"
#include "Daemon.h"

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "generate help message")
      ("cfg-file,c", po::value<std::string>()->default_value("config.xml"),
       "config file")
      ("daemon", "run as daemon")
      ("log,l", po::value<std::string>()->default_value("/var/log/avr_controller.log"),
       "log file")
      ("pidfile,P", po::value<std::string>()->default_value("/var/run/avr_controller.pid"),
       "pid file")
      ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }
    
    bool run_as_daemon = false;
    if (vm.count("daemon"))
      run_as_daemon = true;
    
    boost::asio::io_service io_service;
    
    std::unique_ptr<Daemon> daemon;
    if (run_as_daemon) {
      daemon.reset(new Daemon(io_service, vm["pidfile"].as<std::string>(),
                   vm["log"].as<std::string>()));
    }
    
    AVRController controller(io_service, vm["cfg-file"].as<std::string>());
    CommandAcceptor acceptor(io_service, controller);
    
    io_service.run();
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
