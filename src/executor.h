
#ifndef EXECUTOR_INCLUDED
#define EXECUTOR_INCLUDED

#include "command.h"
#include "signals.h"

enum class executor_status_t
{
	invalid = -1,
	halt,
	busy,
	empty,
	normal,
	last
};

class executor_t
{
public:
	executor_t(command_queue_t& queue);
	~executor_t();

	executor_status_t execute();

private:
	bool halt_, busy_;
	command_queue_t& queue_;

	boost::signals2::connection conn_api_handled;

	void on_api_handled(bool status, const api_t& api, const Json::Value& json);
};

#endif /* EXECUTOR_INCLUDED */
