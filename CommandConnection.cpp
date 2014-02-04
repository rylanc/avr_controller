#include "CommandConnection.h"
#include "AVRController.h"

#include <istream>
#include <iostream>

#include <boost/asio/read_until.hpp>

void CommandConnection::start()
{
  auto sh_this = shared_from_this();
  boost::asio::async_read_until(m_socket, m_read_buf, '\n',
    [=](const boost::system::error_code &err, size_t size) { sh_this->handle_read(err, size); });
}

void CommandConnection::handle_read(const boost::system::error_code &error, size_t size)
{
  if (error)
    return;

  std::istream is(&m_read_buf);
  std::string command;
  std::getline(is, command);

  std::cout << "Received command: " << command << std::endl;

  if (command == "PLAY")
    m_controller.on_play();
  else if (command == "STOP")
    m_controller.on_stop();
  else
    std::cerr << "Ignoring unrecognized command: " << command << std::endl;

  start();
}


CommandAcceptor::CommandAcceptor(boost::asio::io_service &io_service,
				                     AVRController &controller)
  : m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(),
					                                           6666)),
    m_controller(controller)
{
  start_accept();
}

void CommandAcceptor::start_accept()
{
  auto new_connection = CommandConnection::create(m_acceptor.get_io_service(), m_controller);
    
  m_acceptor.async_accept(new_connection->get_socket(),
    [=](const boost::system::error_code& err) { handle_accept(new_connection, err); });
}

void CommandAcceptor::handle_accept(CommandConnection::Ptr new_connection,
                                    const boost::system::error_code& error)
{
  if (!error)
    new_connection->start();

  start_accept();
}
