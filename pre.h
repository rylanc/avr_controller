#include <cstdio>
#include <syslog.h>

#include <chrono>
#include <iostream>
#include <list>
#include <system_error>
#include <thread>
#include <unordered_map>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/optional/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
