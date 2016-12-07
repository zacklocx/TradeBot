
#include "analyzer.h"

#include <boost/bind.hpp>

analyzer_t::analyzer_t(client_t& client, command_queue_t& queue) : client_(client), queue_(queue)
{
	conn_api_handled = sig_api_handled.connect(1, boost::bind(&analyzer_t::on_api_handled, this, _1, _2, _3));
}

analyzer_t::~analyzer_t()
{
	conn_api_handled.disconnect();
}

void analyzer_t::on_api_handled(bool status, const api_t& api, const Json::Value& json)
{
	if(!status)
	{
		return;
	}

	queue_.push(client_.set(api).set(client_));
}
