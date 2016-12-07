
#ifndef SIGNALS_INCLUDED
#define SIGNALS_INCLUDED

#include <boost/signals2.hpp>

#include <json/json.h>

extern boost::signals2::signal<void(bool, const Json::Value&)> sig_command_handled;

#endif /* SIGNALS_INCLUDED */
