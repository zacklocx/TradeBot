
#include <string>
#include <exception>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "client.h"

void dump_json(const Json::Value& data, const std::string& tag = "")
{
	dump_helper_t _(tag);
	LLOG() << data.toStyledString();
}

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		client_t client(service);

		Json::Value data;
		boost::system::error_code ec;

		api_t api;

		{
			param_type param;

			param["symbol"] = "btc_cny";

			api.update_url("https://www.okcoin.cn/api/v1/ticker.do");
			api.update_method("GET");
			api.update_param(param);
		}

		data.clear();
		client.call(api, ec, data);
		dump_json(data, api.url());

		{
			param_type param;

			api.update_url("https://www.okcoin.cn/api/v1/userinfo.do");
			api.update_method("POST");
			api.update_param(param);
		}

		data.clear();
		client.call(api, ec, data);
		dump_json(data, api.url());

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG() << "exception: " << e.what();
	}

	return 0;
}
