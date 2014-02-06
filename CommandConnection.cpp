#include "pre.h"
#include "CommandConnection.h"
#include "AVRController.h"

#include <istream>
#include <iostream>

#include <boost/asio/read_until.hpp>

void CommandConnection::start()
{
  auto sh_this = shared_from_this();
  boost::asio::async_read_until(m_socket, m_read_buf, '\n',
    [=](const boost::system::error_code &err, size_t) { sh_this->handle_read(err); });
}

void CommandConnection::handle_read(const boost::system::error_code &error)
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
  
  
  boost::asio::write(m_socket, boost::asio::buffer("!"));

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
  if (error)
    std::cerr << "Command socket accept failed: " << error.message() << std::endl;
  else
    new_connection->start();

  start_accept();
}



CommandConnection2::CommandConnection2(boost::asio::io_service &io_service,
				                     AVRController &controller)
  : m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(),
					                                           6666)),
    m_socket(io_service), m_controller(controller)
{
  start_accept();
}

void CommandConnection2::start_accept()
{
  m_acceptor.async_accept(m_socket,
    [=](const boost::system::error_code& err) { handle_accept(err); });
}

void CommandConnection2::start_read()
{
  boost::asio::async_read_until(m_socket, m_read_buf, '\n',
    [=](const boost::system::error_code &err, size_t) { handle_read(err); });
}

void CommandConnection2::handle_accept(const boost::system::error_code& error)
{
  if (error)
    std::cerr << "Command socket accept failed: " << error.message() << std::endl;
  else
    start_read();
}

void CommandConnection2::handle_read(const boost::system::error_code &error)
{
  if (error) {
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

  start_read();
}