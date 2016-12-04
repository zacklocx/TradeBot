
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <array>
#include <string>
#include <memory>
#include <functional>

#include <json/json.h>

#include "avhttp.hpp"

#include "api.h"

class client_t
{
public:
	typedef std::function<void(bool, const Json::Value&)> handler_type;

	client_t(boost::asio::io_service& service) : stream_(std::make_shared<avhttp::http_stream>(service))
	{
		buffer_ = std::make_shared<std::array<char, BUFFER_SIZE>>();
	}

	bool call(const api_t& api, Json::Value& json);
	void async_call(const api_t& api, handler_type handler);

private:
	std::shared_ptr<avhttp::http_stream> stream_;

	handler_type handler_;

	std::string data_;
	std::shared_ptr<std::array<char, BUFFER_SIZE>> buffer_;

	static const int BUFFER_SIZE = 1024;

	void handle_open(const boost::system::error_code& ec);
	void handle_read(int bytes_transferred, const boost::system::error_code& ec);
};

#endif /* CLIENT_INCLUDED */
