
#include "client.h"

int main(int argc, char** argv)
{
	boost::asio::io_service service;

	Json::Value json;
	boost::system::error_code ec;

	client_t c(service);
	json = c.call(api_t(), ec);

	service.run();

	return 0;
}
