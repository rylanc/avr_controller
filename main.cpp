#include "CommandConnection.h"
#include "AVRController.h"
#include <iostream>

int main()
{
  try {
    boost::asio::io_service io_service;
    AVRController controller(io_service);
    CommandAcceptor acceptor(io_service, controller);
    io_service.run();
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
