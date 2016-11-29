
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <json/json.h>

#include "avhttp.hpp"

#include "api.h"

class client_t
{
public:
	client_t(boost::asio::io_service& service) : stream_(service) {}

	void call(const api_t& api, boost::system::error_code& ec, Json::Value& data);

private:
	avhttp::http_stream stream_;
};

#endif /* CLIENT_INCLUDED */
