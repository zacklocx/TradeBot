
#include "client.h"

#include <iterator>
#include <algorithm>
#include <exception>

#include "dump.h"
#include "signals.h"

client_t::client_t(boost::asio::io_service& service) :
	stream_(std::make_shared<avhttp::http_stream>(std::ref(service))),
	priority_(0)
{
	data_ = std::make_shared<std::string>();
	buffer_ = std::make_shared<std::array<char, BUFFER_SIZE>>();
}

bool client_t::call(const api_t& api, Json::Value& json)
{
	clean();

	api_ = api;

	boost::system::error_code ec;

	std::string method = api_.method();

	if("GET" == method)
	{
		std::string query = avhttp::map_to_query(api_.param());
		std::string full_url = api_.url() + "?" + query;

		stream_->open(full_url, ec);
	}
	else if("POST" == method)
	{
		avhttp::post_form(*stream_, api_.sign());
		stream_->open(api_.url(), ec);
	}
	else
	{
		throw std::logic_error("bad api method");
	}

	while(!ec)
	{
		std::size_t bytes_transferred = stream_->read_some(boost::asio::buffer(*buffer_), ec);
		std::copy(buffer_->begin(), buffer_->begin() + bytes_transferred, std::back_inserter(*data_));
	}

	stream_->close();

	bool status = false;

	if(boost::asio::error::eof == ec ||
		(boost::asio::error::get_ssl_category() == ec.category() &&
			ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ) == ec.value()))
	{
		status = true;

		Json::Reader reader;
		reader.parse(*data_, json);
	}
	else
	{
		LLOG() << "client error: " << ec.message();
	}

	return status;
}

void client_t::async_call(const api_t& api)
{
	clean();

	api_ = api;

	std::string method = api_.method();

	if("GET" == method)
	{
		std::string query = avhttp::map_to_query(api_.param());
		std::string full_url = api_.url() + "?" + query;

		stream_->async_open(full_url, *this);
	}
	else if("POST" == method)
	{
		avhttp::post_form(*stream_, api_.sign());
		stream_->async_open(api_.url(), *this);
	}
	else
	{
		throw std::logic_error("bad api method");
	}
}

void client_t::operator()(const boost::system::error_code& ec)
{
	if(!ec)
	{
		stream_->async_read_some(boost::asio::buffer(*buffer_), *this);
	}
	else
	{
		LLOG() << "client error: " << ec.message();

		stream_->close();
		sig_api_handled(false, api_, Json::Value::null);
	}
}

void client_t::operator()(const boost::system::error_code& ec, int bytes_transferred)
{
	if(!ec)
	{
		if(bytes_transferred > 0)
		{
			std::copy(buffer_->begin(), buffer_->begin() + bytes_transferred, std::back_inserter(*data_));
			stream_->async_read_some(boost::asio::buffer(*buffer_), *this);
		}
		else
		{
			stream_->close();
			sig_api_handled(true, api_, Json::Value::null);
		}
	}
	else
	{
		stream_->close();

		bool status = false;
		Json::Value json = Json::Value::null;

		if(boost::asio::error::eof == ec ||
			(boost::asio::error::get_ssl_category() == ec.category() &&
				ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ) == ec.value()))
		{
			status = true;

			Json::Reader reader;
			reader.parse(*data_, json);
		}
		else
		{
			LLOG() << "client error: " << ec.message();
		}

		sig_api_handled(status, api_, json);
	}
}

void client_t::clean()
{
	stream_->request_options(avhttp::request_opts());

	*data_ = "";
}
