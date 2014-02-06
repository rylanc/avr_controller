#pragma once

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/streambuf.hpp>

class AVRController;

class CommandConnection
{
public:
  CommandConnection(boost::asio::io_service &io_service, AVRController &controller,
                    const std::string &sock_name);

private:
  void start_accept();
  void start_read();

  void handle_accept(const boost::system::error_code& error);
  void handle_read(const boost::system::error_code &error);

  boost::asio::local::stream_protocol::acceptor m_acceptor;
  boost::asio::local::stream_protocol::socket m_socket;
  boost::asio::streambuf m_read_buf;
  AVRController &m_controller;
};