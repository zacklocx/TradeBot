
#include "executor.h"

#include <boost/bind.hpp>

executor_t::executor_t(command_queue_t& queue) : halt_(false), busy_(false), queue_(queue)
{
	conn_api_handled = sig_api_handled.connect(boost::bind(&executor_t::on_api_handled, this, _1, _2, _3));
}

executor_t::~executor_t()
{
	conn_api_handled.disconnect();
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

void executor_t::on_api_handled(bool status, const api_t& api, const Json::Value& json)
{
	if(!status)
	{
		halt_ = true;
	}

	busy_ = false;
}
