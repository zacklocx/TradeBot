
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <string>
#include <functional>

#include <json/json.h>

#include "avhttp.hpp"

#include "api.h"

class client_t
{
public:
	typedef std::function<void(bool, const Json::Value&)> handler_type;

	client_t(boost::asio::io_service& service) : stream_(service) {}

	bool call(const api_t& api, Json::Value& json);
	void async_call(const api_t& api, handler_type handler);

private:
	avhttp::http_stream stream_;

	handler_type handler_;

	std::string data_;
	boost::array<char, 1024> buffer_;

	void handle_open(const boost::system::error_code& ec);
	void handle_read(int bytes_transferred, const boost::system::error_code& ec);
};

#endif /* CLIENT_INCLUDED */
