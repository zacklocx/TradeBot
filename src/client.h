
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <json/json.h>

#include "avhttp.hpp"

#include "api.h"

class client_t
{
public:
	client_t(boost::asio::io_service& service) : stream_(service) {}

	Json::Value call(const api_t& api, const boost::system::error_code& ec);

private:
	avhttp::http_stream stream_;
};

#endif /* CLIENT_INCLUDED */
