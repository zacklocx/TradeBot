
#include <string>
#include <exception>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "timer.h"
#include "client.h"
#include "executor.h"

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

		command_queue_t q;

		q.push(client.set(-1).set(ticker_api).set(client));
		q.push(client.set(userinfo_api).set(client));

		executor_t executor(q);

		timer_t timer(service, 100, [&]()
		{
			executor_status_t status = executor.execute();

			if(executor_status_t::halt == status || executor_status_t::empty == status)
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
