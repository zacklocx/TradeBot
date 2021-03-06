
#include "signals.h"

boost::signals2::signal<void()> sig_render;

boost::signals2::signal<void(const api_t&, int priority)> sig_api_created;
boost::signals2::signal<void(bool, const api_t&, const Json::Value&)> sig_api_handled;
