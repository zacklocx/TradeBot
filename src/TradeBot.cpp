
#include <string>
#include <exception>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "timer.h"
#include "client.h"
#include "api_command.h"

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

		bool busy = false;

		command_object_t o(0);
		command_queue_t q;

		q.push(api_command_t(ticker_api, client, [&](bool status, const Json::Value& json)
		{
			if(status)
			{
				dump_json(json, ticker_api.url());
				busy = false;
			}
		}));

		q.push(api_command_t(userinfo_api, client, [&](bool status, const Json::Value& json)
		{
			if(status)
			{
				dump_json(json, userinfo_api.url());
				busy = false;
			}
		}));

		timer_t timer(service, 100, [&]()
		{
			if(busy)
			{
				return;
			}

			if(!q.empty())
			{
				busy = true;

				o = q.top();
				q.pop();
				execute(o);
			}
			else
			{
				timer.stop();
			}
		});

		timer.start();

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG() << "exception: " << e.what();
	}

	return 0;
}
