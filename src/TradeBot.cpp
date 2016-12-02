
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

		api_t ticker_api;

		{
			api_t::param_type param;

			param["symbol"] = "btc_cny";

			ticker_api.update_url("https://www.okcoin.cn/api/v1/ticker.do");
			ticker_api.update_method("GET");
			ticker_api.update_param(param);
		}

		api_t userinfo_api;

		{
			api_t::param_type param;

			userinfo_api.update_url("https://www.okcoin.cn/api/v1/userinfo.do");
			userinfo_api.update_method("POST");
			userinfo_api.update_param(param);
		}

		// Json::Value json;

		// json.clear();
		// if(client.call(ticker_api, json))
		// {
		// 	dump_json(json, ticker_api.url());

		// 	json.clear();
		// 	if(client.call(userinfo_api, json))
		// 	{
		// 		dump_json(json, userinfo_api.url());
		// 	}
		// }

		client.async_call(ticker_api, [&](bool status, const Json::Value& json)
		{
			if(status)
			{
				dump_json(json, ticker_api.url());

				client.async_call(userinfo_api, [&](bool status, const Json::Value& json)
				{
					if(status)
					{
						dump_json(json, userinfo_api.url());
					}
				});
			}
		});

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG() << "exception: " << e.what();
	}

	return 0;
}
