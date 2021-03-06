
#include <thread>
#include <exception>
#include <initializer_list>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "client.h"
#include "renderer.h"
#include "analyzer.h"
#include "executor.h"
#include "generator.h"
#include "callback_timer.h"

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		client_t client(service);

		command_queue_t queue;

		analyzer_t analyzer;
		analyzer.init_modules();

		executor_t executor(queue);
		generator_t generator(client, queue);

		api_t kline_api("kline", "GET", {{"symbol", "btc_cny"}, {"type", "1min"}, {"size", "100"}});
		api_t depth_api("depth", "GET", {{"symbol", "btc_cny"}, {"size", "100"}, {"merge", "0.1"}});
		api_t ticker_api("ticker", "GET", {{"symbol", "btc_cny"}});
		api_t userinfo_api("userinfo", "POST");

		generator.generate(kline_api);
		generator.generate(depth_api);
		generator.generate(ticker_api);
		generator.generate(userinfo_api);

		int peroid = 100;

		callback_timer_t timer(service, peroid, [&]
		{
			executor_status_t status = executor.execute();

			if(executor_status_t::invalid == status ||
				executor_status_t::halt == status ||
				executor_status_t::empty == status)
			{
				timer.stop();
			}
		});

		timer.start();

		std::thread event_thread([&] { service.run(); });

		renderer_t::start(1224, 800, 0xA6A6A6);

		event_thread.join();
	}
	catch(std::exception& e)
	{
		LLOG() << "exception: " << e.what();
	}

	return 0;
}
