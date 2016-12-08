
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

void analyzer_t::on_api_handled(bool status, const api_t& api, const Json::Value& json)
{
	if(!status || json.isNull())
	{
		return;
	}

	std::string name = api.name();

	if("userinfo" == name)
	{
		bool result = jtob(json, "result");

		if(!result)
		{
			sig_api_created(api, 0);
		}
		else
		{
			double cny = jtod(json, "info.funds.free.cny");
			double btc = jtod(json, "info.funds.free.btc");

			LLOG() << "cny: " << cny;
			LLOG() << "btc: " << btc;
		}
	}
	else if("ticker" == name)
	{
		uint64_t ts = jtou64(json, "date");

		double low = jtod(json, "ticker.low");
		double high = jtod(json, "ticker.high");
		double last = jtod(json, "ticker.last");

		LLOG() << "ts: " << ts;
		LLOG() << "low: " << low;
		LLOG() << "high: " << high;
		LLOG() << "last: " << last;
	}
}
