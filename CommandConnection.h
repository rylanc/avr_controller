#pragma once

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

class AVRController;

class CommandConnection : public std::enable_shared_from_this<CommandConnection>
{
public:
  typedef std::shared_ptr<CommandConnection> Ptr;

  static Ptr create(boost::asio::io_service &io_service, AVRController &controller)
  {
    return Ptr(new CommandConnection(io_service, controller));
  }

  void start();

  boost::asio::ip::tcp::socket &get_socket() { return m_socket; }

private:
  CommandConnection(boost::asio::io_service &io_service, AVRController &controller)
    : m_socket(io_service), m_controller(controller) {}

  void handle_read(const boost::system::error_code &error, size_t size);

  boost::asio::ip::tcp::socket m_socket;
  boost::asio::streambuf m_read_buf;
  AVRController &m_controller;
};

class CommandAcceptor
{
public:
  CommandAcceptor(boost::asio::io_service &io_service, AVRController &controller);

private:
  void start_accept();

  void handle_accept(CommandConnection::Ptr new_connection,
                     const boost::system::error_code& error);

  boost::asio::ip::tcp::acceptor m_acceptor;
  AVRController &m_controller;
};
