#include "CommandConnection.h"
#include "AVRController.h"

#include <functional>
#include <istream>

#include <boost/asio/read_until.hpp>

#include <iostream>

using namespace std;

void CommandConnection::start()
{
  boost::asio::async_read_until(m_socket, m_read_buf, '\n',
				std::bind(&CommandConnection::handle_read,
					  shared_from_this(),
					  std::placeholders::_1, std::placeholders::_2));
}

void CommandConnection::handle_read(const boost::system::error_code &error, size_t size)
{
  if (error)
    return;

  std::istream is(&m_read_buf);
  std::string command;
  std::getline(is, command);

  cout << "Received command: " << command << endl;

  if (command == "PLAY")
    m_controller.on_play();
  else if (command == "STOP")
    m_controller.on_stop();
  else
    cerr << "Ignoring unrecognized command: " << command << endl;

  start();
}


CommandAcceptor::CommandAcceptor(boost::asio::io_service &io_service,
				 AVRController &controller)
  : m_acceptor(io_service,
	       boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(),
					      6666)),
    m_controller(controller)
{
  start_accept();
}

void CommandAcceptor::start_accept()
{
  CommandConnection::Ptr new_connection =
    CommandConnection::create(m_acceptor.get_io_service(), m_controller);

  m_acceptor.async_accept(new_connection->get_socket(),
    std::bind(&CommandAcceptor::handle_accept, this, new_connection,
    std::placeholders::_1));
}

void CommandAcceptor::handle_accept(CommandConnection::Ptr new_connection,
  const boost::system::error_code& error)
{
  if (!error)
    new_connection->start();

  start_accept();
}
