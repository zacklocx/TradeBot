
#include <initializer_list>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "utils.h"
#include "client.h"

using boost::asio::ip::tcp;

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		std::string host = "www.okcoin.cn";

		tcp::resolver resolver(service);
		tcp::resolver::query query(host, "https");
		tcp::resolver::iterator endpoint = resolver.resolve(query);

		client_t c(service, context, endpoint);
		c.request(api_t(host, "/api/v1/ticker", "GET", {{"symbol", "btc_cny"}}));

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG(false) << "Exception: " << e.what();
	}

	return 0;
}
