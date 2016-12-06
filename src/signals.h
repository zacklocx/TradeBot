
#ifndef SIGNALS_INCLUDED
#define SIGNALS_INCLUDED

#include <boost/signals2.hpp>

#include <json/json.h>

extern boost::signals2::signal<void(const Json::Value&)> sig_command_finish;
extern boost::signals2::signal<void(const Json::Value&)> sig_command_fail;

#endif /* SIGNALS_INCLUDED */
