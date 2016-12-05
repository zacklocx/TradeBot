
#include "command_signals.h"

boost::signals2::signal<void(const Json::Value&)> sig_command_finish;
boost::signals2::signal<void(const Json::Value&)> sig_command_fail;
