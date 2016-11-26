
#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/coroutine.hpp>

#include <json/json.h>

#include "api.h"

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef std::shared_ptr<boost::asio::streambuf> asio_streambuf_ptr;

class client_t : boost::asio::coroutine
{
public:
	client_t(boost::asio::io_service& service,
		boost::asio::ssl::context& context,
		tcp::resolver::iterator endpoint):
		socket_(std::make_shared<ssl::stream<tcp::socket>>(service, context)),
		endpoint_(endpoint)
	{}

	void request(const api_t& api);

	void operator()(const boost::system::error_code& ec, size_t bt = 0);

private:
	std::shared_ptr<ssl::stream<tcp::socket>> socket_;
	tcp::resolver::iterator endpoint_;
	asio_streambuf_ptr request_;
	asio_streambuf_ptr response_;

	api_t api_;
};

#endif /* CLIENT_INCLUDED */
