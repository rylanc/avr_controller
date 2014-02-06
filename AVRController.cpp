#include "pre.h"
#include "AVRController.h"
#include "ConfigTranslators.h"
#include <chrono>
#include <iostream>
#include <thread>

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;

AVRController::AVRController(boost::asio::io_service &io_service,
                             const std::string &cfg_file)
  : m_serial_port(io_service), m_stop_timer(io_service)
{
  load_config(cfg_file);
  request_current_state();
  start_read();
}

void AVRController::load_config(const std::string &cfg_file)
{
  using boost::property_tree::ptree;
  using boost::asio::serial_port;

  ptree pt;
  read_xml(cfg_file, pt);

  string device = pt.get<string>("avr_controller.serial_settings.<xmlattr>.device");
  m_serial_port.open(device);

  unsigned int baud_rate =
    pt.get<unsigned int>("avr_controller.serial_settings.<xmlattr>.baud_rate");
  if (baud_rate)
    m_serial_port.set_option(serial_port::baud_rate(baud_rate));

  boost::optional<serial_port::parity::type> parity =
    pt.get_optional<serial_port::parity::type>("avr_controller.serial_settings.<xmlattr>.parity");
  if (parity)
    m_serial_port.set_option(serial_port::parity(*parity));

  boost::optional<serial_port::stop_bits::type> stop_bits =
    pt.get_optional<serial_port::stop_bits::type>("avr_controller.serial_settings.<xmlattr>.stop_bits");
  if (stop_bits)
    m_serial_port.set_option(serial_port::stop_bits(*stop_bits));

  boost::optional<unsigned int> character_size =
    pt.get_optional<unsigned int>("avr_controller.serial_settings.<xmlattr>.character_size");
  if (character_size)
    m_serial_port.set_option(serial_port::character_size(*character_size));

  for (auto &child : pt.get_child("avr_controller.play_state")) {
    string command = child.second.get<string>("<xmlattr>.command");
    string value = child.second.get<string>("<xmlattr>.value");
    size_t delay = child.second.get<size_t>("<xmlattr>.delay", 20);
    m_play_state.push_back({command, value, delay});
    m_current_state[command] = "";
  }

  m_stop_timer_duration =
    pt.get<size_t>("avr_controller.stop_state.<xmlattr>.seconds_to_wait", 90);

  for (auto &child : pt.get_child("avr_controller.stop_state")) {
    if (child.first == "state") {
      string command = child.second.get<string>("<xmlattr>.command");
      string value = child.second.get<string>("<xmlattr>.value");
      size_t delay = child.second.get<size_t>("<xmlattr>.delay", 20);
      m_stop_state.push_back({command, value, delay});
      m_current_state[command] = "";
    }
    else if (child.first == "cancel_if") {
      string event = child.second.get<string>("<xmlattr>.event");
      m_cancel_stop_events.push_back(event);
    }
  }  
}

void AVRController::request_current_state()
{
  for (auto &state : m_current_state) { 
    string req = state.first + "?\r";
    cout << "Sending: " << req << endl;
    boost::system::error_code error;
    boost::asio::write(m_serial_port, boost::asio::buffer(req), error);
    if (error)
      std::cerr << "Serial port write failed: " << error.message() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}

void AVRController::on_play()
{
  m_stop_timer.cancel();
  set_avr_state(m_play_state);
}

void AVRController::on_stop()
{
  m_stop_timer.expires_from_now(std::chrono::seconds(m_stop_timer_duration));
  m_stop_timer.async_wait([=](const boost::system::error_code& err) { on_stop_timer(err); });
}

void AVRController::on_stop_timer(const boost::system::error_code& error)
{
  if (error) {
    std::cerr << "Stop timer failed: " << error.message() << std::endl;
    return;
  }

  set_avr_state(m_stop_state);
}

void AVRController::set_avr_state(const StateList &state_list)
{
  for (auto &state : state_list) {
    const string &current_value = m_current_state[state.command];
    if (state.value != current_value) {
      string command = state.command + state.value + "\r";
      cout << "Sending (" << state.delay << "): " << command << endl;
      boost::system::error_code error;
      boost::asio::write(m_serial_port, boost::asio::buffer(command), error);
      if (error)
        std::cerr << "Serial port write failed: " << error.message() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(state.delay));
    }
  }
}

void AVRController::start_read()
{
  boost::asio::async_read_until(m_serial_port, m_read_buf, '\r',
    [=](const boost::system::error_code &err, size_t) { handle_read(err); });
}

void AVRController::handle_read(const boost::system::error_code &error)
{
  if (error) {
    std::cerr << "Serial port read failed: " << error.message() << std::endl;
    return;
  }

  cout << "Timer expires: " << m_stop_timer.expires_from_now().count() << endl;

  std::istream is(&m_read_buf);
  std::string rsp;
  std::getline(is, rsp, '\r');

  cout << "Received: " << rsp << endl;
  
  if (m_stop_timer.expires_from_now().count() > 0) {
    if (std::any_of(m_cancel_stop_events.begin(), m_cancel_stop_events.end(),
                    [&](const string &event) { return boost::starts_with(rsp, event); })) {
      cout << "Cancelling timer" << endl;
      m_stop_timer.cancel();
    }
  }

  handle_response(rsp);

  start_read();
}

void AVRController::handle_response(const std::string &rsp)
{
  // TODO: Is there a generic way to do this?
  if (rsp.find(' ') != string::npos)
    return;

  auto it = std::find_if(m_current_state.begin(), m_current_state.end(),
                         [&](const StateMap::value_type& state)
                         { return boost::starts_with(rsp, state.first); });
  if (it != m_current_state.end())
    it->second = rsp.substr(it->first.length());
}
