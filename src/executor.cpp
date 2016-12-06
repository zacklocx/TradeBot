
#include "executor.h"

#include <boost/bind.hpp>

executor_t::executor_t(command_queue_t& queue) : halt_(false), busy_(false), queue_(queue)
{
	conn_command_finish = sig_command_finish.connect(boost::bind(&executor_t::on_command_finish, this, _1));
	conn_command_fail = sig_command_fail.connect(boost::bind(&executor_t::on_command_fail, this, _1));
}

executor_t::~executor_t()
{
	conn_command_finish.disconnect();
	conn_command_fail.disconnect();
}

executor_status_t executor_t::execute()
{
	executor_status_t ret = executor_status_t::invalid;

	if(halt_)
	{
		ret = executor_status_t::halt;
	}
	else if(busy_)
	{
		ret = executor_status_t::busy;
	}
	else if(queue_.empty())
	{
		ret = executor_status_t::empty;
	}
	else
	{
		busy_ = true;

		auto command = queue_.top();
		queue_.pop();
		::execute(command);

		ret = executor_status_t::normal;
	}

	return ret;
}

void executor_t::on_command_finish(const Json::Value& json)
{
	busy_ = false;
}

void executor_t::on_command_fail(const Json::Value& json)
{
	halt_ = true;
	busy_ = false;
}
