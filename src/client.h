
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <cstdint>

#include <array>
#include <string>
#include <memory>

#include <json/json.h>

#include "avhttp.hpp"

#include "api.h"

class client_t
{
public:
	client_t(boost::asio::io_service& service);

	api_t api() const { return api_; }
	int64_t priority() const { return priority_; }

	client_t& set(const api_t& api) { api_ = api; return *this; }
	client_t& set(int64_t priority) { priority_ = priority; return *this; }

	bool call(const api_t& api, Json::Value& json);
	void async_call(const api_t& api);

	void operator()(const boost::system::error_code& ec);
	void operator()(const boost::system::error_code& ec, int bytes_transferred);

private:
	static const int BUFFER_SIZE = 1024;

	std::shared_ptr<avhttp::http_stream> stream_;

	std::shared_ptr<std::string> data_;
	std::shared_ptr<std::array<char, BUFFER_SIZE>> buffer_;

	api_t api_;
	int64_t priority_;

	void clean();
};

inline int64_t priority(const client_t& client) { return client.priority(); }
inline void execute(client_t& client) { client.async_call(client.api()); }

#endif /* CLIENT_INCLUDED */
