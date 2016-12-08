
#ifndef SIGNALS_INCLUDED
#define SIGNALS_INCLUDED

#include <boost/signals2.hpp>

#include <json/json.h>

#include "api.h"

extern boost::signals2::signal<void(const api_t&, int priority)> sig_api_created;
extern boost::signals2::signal<void(bool, const api_t&, const Json::Value&)> sig_api_handled;

#endif /* SIGNALS_INCLUDED */
