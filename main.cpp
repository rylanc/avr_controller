#include "pre.h"
#include "CommandConnection.h"
#include "AVRController.h"
#include "Daemon.h"

#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  try {
    po::options_description desc("Options");
    desc.add_options()
      ("help,h", "Display this information")
      ("cfg-file,c", po::value<std::string>()->default_value("/etc/avr_controller.xml"),
       "Use this file for configuration")
      ("daemon", "Run as a daemon")
      ("log,l", po::value<std::string>()->default_value("/var/log/avr_controller.log"),
       "Redirect all output to a file")
      ("pidfile,P", po::value<std::string>()->default_value("/var/run/avr_controller.pid"),
       "Write daemon's pid to a file on startup")
      ("socket-name,s", po::value<std::string>()->default_value("/var/run/avr_controller.sock"),
       "Location of UNIX domain socket used for issuing commands")
      ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }
    
    boost::asio::io_service io_service;
    
    std::unique_ptr<Daemon> daemon;
    if (vm.count("daemon")) {
      daemon.reset(new Daemon(io_service, vm["pidfile"].as<std::string>(),
                              vm["log"].as<std::string>()));
    }
    
    AVRController controller(io_service, vm["cfg-file"].as<std::string>());
    CommandConnection connection(io_service, controller,
                                 vm["socket-name"].as<std::string>());
    
    io_service.run();
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
