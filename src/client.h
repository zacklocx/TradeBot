
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

	client_t(boost::asio::io_service& service);

	client_t& set(const api_t& api) { api_ = api; return *this; }
	client_t& set(int priority) { priority_ = priority; return *this; }
	client_t& set(handler_type handler) { handler_ = handler; return *this; }

	int priority() const { return priority_; }

	bool call(Json::Value& json) { return call(api_, json); }
	void async_call() { async_call(api_, handler_); }

	bool call(const api_t& api, Json::Value& json);
	void async_call(const api_t& api, handler_type handler);

	void operator()(bool status, const Json::Value& json);

	void operator()(const boost::system::error_code& ec);
	void operator()(const boost::system::error_code& ec, int bytes_transferred);

private:
	static const int BUFFER_SIZE = 1024;

	std::shared_ptr<avhttp::http_stream> stream_;

	std::shared_ptr<std::string> data_;
	std::shared_ptr<std::array<char, BUFFER_SIZE>> buffer_;

	api_t api_;
	int priority_;
	handler_type handler_;

	void clean();
};

inline int priority(const client_t& client) { return client.priority(); }
inline void execute(client_t& client) { client.async_call(); }

#endif /* CLIENT_INCLUDED */
