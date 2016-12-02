
#include <string>
#include <exception>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "timer.h"
#include "client.h"
#include "command.h"

void dump_json(const Json::Value& data, const std::string& tag = "")
{
	dump_helper_t _(tag);
	LLOG() << data.toStyledString();
}

////////////////////////////////////////////////////////////////////////////////
class command_block_t
{
public:
	command_block_t(int i = 0) : i_(i) {}

	friend int priority(const command_block_t& o);
	friend void execute(command_block_t& o);

private:
	int i_;

	void print() const { LLOG() << i_; }
};

int priority(const command_block_t& o)
{
	return o.i_;
}

void execute(command_block_t& o)
{
	return o.print();
}
////////////////////////////////////////////////////////////////////////////////
class call_api_block_t
{
public:
	call_api_block_t(client_t& client, const api_t& api, bool& busy) :
		client_(client), api_(api), busy_(busy)
	{}

	void execute()
	{
		busy_ = true;

		client_.async_call(api_, [&](bool status, const Json::Value& json)
		{
			if(status)
			{
				dump_json(json, api_.url());
				busy_ = false;
			}
		});
	}

private:
	client_t& client_;
	const api_t& api_;
	bool& busy_;
};

int priority(const call_api_block_t& o)
{
	return 100;
}

void execute(call_api_block_t& o)
{
	o.execute();
}

////////////////////////////////////////////////////////////////////////////////

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

		// client.async_call(ticker_api, [&](bool status, const Json::Value& json)
		// {
		// 	if(status)
		// 	{
		// 		dump_json(json, ticker_api.url());

		// 		client.async_call(userinfo_api, [&](bool status, const Json::Value& json)
		// 		{
		// 			if(status)
		// 			{
		// 				dump_json(json, userinfo_api.url());
		// 			}
		// 		});
		// 	}
		// });

		bool busy = false;

		command_object_t o(0);
		command_queue_t q;

		q.push(command_block_t(3));
		q.push(command_block_t(4));
		q.push(command_block_t(5));

		q.push(call_api_block_t(client, ticker_api, busy));
		q.push(call_api_block_t(client, userinfo_api, busy));

		timer_t timer(service, 100, [&]()
		{
			if(busy)
			{
				return;
			}

			if(!q.empty())
			{
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
