
#include "client.h"

#include <boost/asio/yield.hpp>

#include "dump.h"

void dump_json(const Json::Value& v, const std::string& tag)
{
	dump_helper_t _(tag);
	LLOG(false) << v.toStyledString();
}

std::string buffer_to_string(const boost::asio::streambuf& buf)
{
	using boost::asio::buffers_begin;

	auto data = buf.data();
	std::string ret(buffers_begin(data), buffers_begin(data) + buf.size());

	return ret;
}

void client_t::request(const api_t& api)
{
	api_ = api;

	boost::system::error_code ec;

	operator()(ec, 0);
}

void client_t::operator()(const boost::system::error_code& ec, size_t bt /* = 0 */)
{
	if(!ec)
	{
		reenter(this)
		{
			for(;;)
			{
				yield boost::asio::async_connect(socket_->lowest_layer(), endpoint_, boost::bind(&client_t::operator(), this, boost::asio::placeholders::error, 0));
				yield socket_->async_handshake(boost::asio::ssl::stream_base::client, *this);
				yield
				{
					std::ostream request_stream(&*request_);
					request_stream << api_.prepare();
					boost::asio::async_write(*socket_, *request_, *this);
				}
				yield boost::asio::async_read_until(*socket_, *response_, "\r\n", *this);
			}
		}
	}
	else
	{
		LLOG(false) << ec.message();
	}
}
