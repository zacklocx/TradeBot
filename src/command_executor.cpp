
#include "command_executor.h"

command_executor_status_t command_executor_t::execute()
{
	command_executor_status_t ret = command_executor_status_t::invalid;

	if(halt_)
	{
		ret = command_executor_status_t::halt;
	}
	else if(busy_)
	{
		ret = command_executor_status_t::busy;
	}
	else if(queue_.empty())
	{
		ret = command_executor_status_t::empty;
	}
	else
	{
		busy_ = true;

		auto command = queue_.top();
		queue_.pop();
		::execute(command);

		ret = command_executor_status_t::normal;
	}

	return ret;
}

void command_executor_t::on_command_finish(const Json::Value& json)
{
	busy_ = false;
}

void command_executor_t::on_command_fail(const Json::Value& json)
{
	halt_ = true;
	busy_ = false;
}
