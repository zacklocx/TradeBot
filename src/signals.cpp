
#include "signals.h"

boost::signals2::signal<void(bool, const Json::Value&)> sig_command_handled;
