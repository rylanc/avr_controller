#pragma once

#include <boost/asio/signal_set.hpp>

class Daemon
{
public:
  Daemon(boost::asio::io_service &io_service);
  ~Daemon();
  
  int init();
  
private:
  boost::asio::signal_set m_signals;
  int m_lock_fd;
};
