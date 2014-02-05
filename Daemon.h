#pragma once

#include <boost/asio/signal_set.hpp>

class Daemon
{
public:
  Daemon(boost::asio::io_service &io_service, const std::string &pidfile,
         const std::string &logfile);
  ~Daemon();
  
private:
  boost::asio::signal_set m_signals;
  std::string m_pid_file;
  int m_lock_fd;
};
