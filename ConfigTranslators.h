#pragma once

#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/optional/optional.hpp>

struct parity_translator {
  typedef std::string internal_type;
  typedef boost::asio::serial_port::parity::type external_type;

  boost::optional<external_type> get_value(const internal_type &str)
  {
    if (str == "none")
      return boost::asio::serial_port::parity::none;
    else if (str == "odd")
      return boost::asio::serial_port::parity::odd;
    else if (str == "even")
      return boost::asio::serial_port::parity::even;
    else
      return boost::optional<external_type>();
  }
};

struct stop_bits_translator {
  typedef std::string internal_type;
  typedef boost::asio::serial_port::stop_bits::type external_type;

  boost::optional<external_type> get_value(const internal_type &str)
  {
    if (str == "one")
      return boost::asio::serial_port::stop_bits::one;
    else if (str == "onepointfive")
      return boost::asio::serial_port::stop_bits::onepointfive;
    else if (str == "two")
       return boost::asio::serial_port::stop_bits::two;
    else
       return boost::optional<external_type>();
  }
};

namespace boost {
  namespace property_tree {
    template<>
    struct translator_between<std::string, boost::asio::serial_port::parity::type>
    { typedef parity_translator type; };

    template<>
    struct translator_between<std::string, boost::asio::serial_port::stop_bits::type>
    { typedef stop_bits_translator type; };
  }
}
