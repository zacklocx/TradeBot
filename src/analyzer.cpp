
#include "analyzer.h"

#include <string>

#include <boost/bind.hpp>

#include "utils.h"

analyzer_t::analyzer_t()
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

	std::string name = api.name();

	dump_json(json, name);
}
