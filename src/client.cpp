
#include "client.h"

#include <iterator>
#include <algorithm>
#include <exception>

#include "dump.h"

bool client_t::call(const api_t& api, Json::Value& json)
{
	bool status = false;

	data_ = "";

	boost::system::error_code ec;

	std::string method = api.method();

	if("GET" == method)
	{
		std::string query = avhttp::map_to_query(api.param());
		std::string full_url = api.url() + "?" + query;

		stream_->open(full_url, ec);
	}
	else if("POST" == method)
	{
		avhttp::post_form(*stream_, api.prepare());
		stream_->open(api.url(), ec);
	}
	else
	{
		throw std::logic_error("bad api method");
	}

	while(!ec)
	{
		std::size_t bytes_transferred = stream_->read_some(boost::asio::buffer(*buffer_), ec);
		std::copy(buffer_->begin(), buffer_->begin() + bytes_transferred, std::back_inserter(data_));
	}

	stream_->close();

	if(boost::asio::error::eof == ec ||
		(boost::asio::error::get_ssl_category() == ec.category() &&
			ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ) == ec.value()))
	{
		Json::Reader reader;
		reader.parse(data_, json);

		status = true;
	}
	else
	{
		LLOG() << "client error: " << ec.message();
	}

	return status;
}

void client_t::async_call(const api_t& api, handler_type handler)
{
	handler_ = handler;

	data_ = "";

	std::string method = api.method();

	if("GET" == method)
	{
		std::string query = avhttp::map_to_query(api.param());
		std::string full_url = api.url() + "?" + query;

		stream_->async_open(full_url, boost::bind(&client_t::handle_open, this, boost::asio::placeholders::error));
	}
	else if("POST" == method)
	{
		avhttp::post_form(*stream_, api.prepare());
		stream_->async_open(api.url(), boost::bind(&client_t::handle_open, this, boost::asio::placeholders::error));
	}
	else
	{
		throw std::logic_error("bad api method");
	}
}

void client_t::handle_open(const boost::system::error_code& ec)
{
	if(!ec)
	{
		stream_->async_read_some(boost::asio::buffer(*buffer_),
			boost::bind(&client_t::handle_read, this,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error));
	}
	else
	{
		stream_->close();

		LLOG() << "client error: " << ec.message();

		if(handler_)
		{
			handler_(false, Json::Value());
		}
	}
}

void client_t::handle_read(int bytes_transferred, const boost::system::error_code& ec)
{
	if(!ec)
	{
		std::copy(buffer_->begin(), buffer_->begin() + bytes_transferred, std::back_inserter(data_));

		stream_->async_read_some(boost::asio::buffer(*buffer_),
			boost::bind(&client_t::handle_read, this,
				boost::asio::placeholders::bytes_transferred,
				boost::asio::placeholders::error));
	}
	else
	{
		stream_->close();

		if(handler_)
		{
			bool status = false;

			Json::Value json;

			if(boost::asio::error::eof == ec ||
				(boost::asio::error::get_ssl_category() == ec.category() &&
					ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ) == ec.value()))
			{
				Json::Reader reader;
				reader.parse(data_, json);

				status = true;
			}
			else
			{
				LLOG() << "client error: " << ec.message();
			}

			handler_(status, json);
		}
	}
}
