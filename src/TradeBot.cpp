
#include <string>
#include <exception>
#include <initializer_list>

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

		api_t ticker_api("ticker", "GET", {{"symbol", "btc_cny"}});
		api_t userinfo_api("userinfo", "POST");

		command_queue_t q;

		q.push(client.set(-1).set(ticker_api).set(client));
		q.push(client.set(userinfo_api).set(client));

		executor_t executor(q);

		int peroid = 100;

		timer_t timer(service, peroid, [&]()
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
