#pragma once
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/system_timer.hpp>

#include <unordered_map>
#include <list>

class AVRController
{
public:
  AVRController(boost::asio::io_service &io_service);
  
  void on_play();
  void on_stop();

private:

  struct AVRState
  {
    std::string command;
    std::string value;
    size_t delay;
  };

  typedef std::unordered_map<std::string, std::string> StateMap;
  typedef std::list<AVRState> StateList;

  void load_config();

  void request_current_state();

  void parse_response(const std::string &rsp);

  void set_avr_state(const StateList &state_list);

  void start_read();
  void handle_read(const boost::system::error_code &error, size_t size);
  
  void on_stop_timer(const boost::system::error_code& error);

  boost::asio::serial_port m_serial_port;
  boost::asio::streambuf m_read_buf;
  StateList m_play_state;
  StateList m_stop_state;
  StateMap m_current_state;
  size_t m_stop_timer_duration;
  boost::asio::system_timer m_stop_timer;
  std::list<std::string> m_cancel_stop_events;
};
