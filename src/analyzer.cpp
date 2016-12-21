
#include "analyzer.h"

#include <string>

#include <boost/bind.hpp>

#include "dump.h"
#include "utils.h"

analyzer_t::analyzer_t()
{
	conn_api_handled = sig_api_handled.connect(1, boost::bind(&analyzer_t::on_api_handled, this, _1, _2, _3));
}

analyzer_t::~analyzer_t()
{
	conn_api_handled.disconnect();
}

void analyzer_t::init_modules()
{
	ticker_mod.init(500, 20);
}

void analyzer_t::on_api_handled(bool status, const api_t& api, const Json::Value& json)
{
	if(!status)
	{
		return;
	}

	LLOG(true) << json.toStyledString();

	std::string name = api.name();

	if("ticker" == name)
	{
		float price = jtof(query_json(json, "ticker.last"));

		ticker_mod.analyze(price);

		sig_api_created(api, 0);
	}
}
