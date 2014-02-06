#include "pre.h"
#include "CommandConnection.h"
#include "AVRController.h"

#include <cstdio>
#include <iostream>

#include <boost/asio/read_until.hpp>

CommandConnection::CommandConnection(boost::asio::io_service &io_service,
                                     AVRController &controller,
                                     const std::string &sock_name)
  : m_acceptor(io_service, boost::asio::local::stream_protocol()),
    m_socket(io_service), m_controller(controller)
{
  std::remove(sock_name.c_str());
  m_acceptor.bind(boost::asio::local::stream_protocol::endpoint(sock_name));
  m_acceptor.listen();
  start_accept();
}

void CommandConnection::start_accept()
{
  m_acceptor.async_accept(m_socket,
    [=](const boost::system::error_code& err) { handle_accept(err); });
}

void CommandConnection::start_read()
{
  boost::asio::async_read_until(m_socket, m_read_buf, '\n',
    [=](const boost::system::error_code &err, size_t) { handle_read(err); });
}

void CommandConnection::handle_accept(const boost::system::error_code& error)
{
  if (error)
    std::cerr << "Command socket accept failed: " << error.message() << std::endl;
  else
    start_read();
}

void CommandConnection::handle_read(const boost::system::error_code &error)
{
  if (error) {
    m_socket.close();
    start_accept();
    return;
  }

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
    
  boost::asio::write(m_socket, boost::asio::buffer("!"));

  start_read();
}
