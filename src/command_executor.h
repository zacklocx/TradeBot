
#ifndef COMMAND_EXECUTOR_INCLUDED
#define COMMAND_EXECUTOR_INCLUDED

#include <json/json.h>

#include "command.h"
#include "command_signals.h"

enum class command_executor_status_t
{
	invalid = -1,
	halt,
	busy,
	empty,
	normal,
	last
};

class command_executor_t
{
public:
	command_executor_t(command_queue_t& queue);
	~command_executor_t();

	command_executor_status_t execute();

private:
	bool halt_, busy_;
	command_queue_t& queue_;

	boost::signals2::connection conn_command_finish;
	boost::signals2::connection conn_command_fail;

	void on_command_finish(const Json::Value& json);
	void on_command_fail(const Json::Value& json);
};

#endif /* COMMAND_EXECUTOR_INCLUDED */
