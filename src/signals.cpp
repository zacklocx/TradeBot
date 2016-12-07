
#include "signals.h"

boost::signals2::signal<void(bool, const api_t&, const Json::Value&)> sig_api_handled;
