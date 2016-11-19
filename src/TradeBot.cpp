
#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>

#include "TradeBotConfig.h"
#include "TradeBotUtils.h"

std::string buffer_to_string(const boost::asio::streambuf& buf)
{
	using boost::asio::buffers_begin;
	auto data = buf.data();
	return std::string(buffers_begin(data), buffers_begin(data) + buf.size());
}

class client
{
public:
	client(boost::asio::io_service& service,
			boost::asio::ssl::context& context,
			const std::string& host,
			int period) :
			socket_(service, context),
			host_(host),
			period_(period),
			timer_(service, std::chrono::milliseconds(period))
	{
		init();

		timer_.async_wait(boost::bind(&client::handle_timer, this, boost::asio::placeholders::error));

		socket_.set_verify_mode(boost::asio::ssl::verify_peer);
		socket_.set_verify_callback(boost::bind(&client::verify_certificate, this, _1, _2));

		boost::asio::ip::tcp::resolver resolver(service);
		boost::asio::ip::tcp::resolver::query query(host_, "https");
		endpoint_iterator_ = resolver.resolve(query);
	}

	void init()
	{
		api_key_ = "91a440cb-d0c2-4dd6-924e-320d2a95a543";
		secret_key_ = "8AF27D70C7D51568ADC74FA0CBF707F0";

		is_busy_ = will_halt_ = false;

		round_ = 0;
	}

	void start_connect()
	{
		boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator_,
			boost::bind(&client::handle_connect, this, boost::asio::placeholders::error));
	}

	bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& context)
	{
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));

		LLOG() << "Verifying " << subject_name;
		LLOG();

		return preverified;
	}

	void handle_timer(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			if(!will_halt_)
			{
				if(!is_busy_)
				{
					is_busy_ = true;
					++round_;

					start_connect();
				}

				timer_.expires_at(timer_.expires_at() + std::chrono::milliseconds(period_));
				timer_.async_wait(boost::bind(&client::handle_timer, this, boost::asio::placeholders::error));
			}
		}
		else
		{
			LLOG(std::cerr) << "Timer error: " << ec.message();
		}
	}

	void handle_connect(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			socket_.async_handshake(boost::asio::ssl::stream_base::client,
				boost::bind(&client::handle_handshake, this, boost::asio::placeholders::error));
		}
		else
		{
			LLOG(std::cerr) << "Connect failed: " << ec.message();
		}
	}

	void handle_handshake(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			test_api();
		}
		else
		{
			LLOG(std::cerr) << "Handshake failed: " << ec.message();
		}
	}

	void handle_write(const boost::system::error_code& ec, size_t bt)
	{
		if(!ec)
		{
			boost::asio::async_read_until(socket_, response_, "\r\n",
				boost::bind(&client::handle_read, this,
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			LLOG(std::cerr) << "Write failed: " << ec.message();
		}
	}

	void handle_read(const boost::system::error_code& ec, size_t bt)
	{
		if(!ec)
		{
			Json::Value root;

			if(parse_response(root))
			{
				dump_helper _("dump data");
				dump_value(root);

				is_busy_ = false;
				will_halt_ = true;
			}
		}
		else
		{
			LLOG(std::cerr) << "Read failed: " << ec.message();
		}
	}

	void send_request(const std::string& method,
						const std::string& url,
						std::map<std::string, std::string>& param)
	{
		std::string formatted_method(method);
		std::transform(formatted_method.begin(), formatted_method.end(), formatted_method.begin(),
			[](char c) { return std::toupper(c); });

		if(formatted_method != "GET" && formatted_method != "POST")
		{
			LLOG(std::cerr) << "Unsupported method";
			return;
		}

		std::string separator = "";

		std::ostringstream param_stream;

		if("POST" == formatted_method)
		{
			param_stream << "api_key=" << api_key_ << "&";
		}

		for(const auto& it : param)
		{
			param_stream << separator << it.first << "=" << it.second;
			separator = "&";
		}

		if("POST" == formatted_method)
		{
			std::ostringstream param_with_key;
			param_with_key << param_stream.str() << separator << "secret_key=" << secret_key_;

			std::string sign = md5(param_with_key.str());

			param_stream << separator << "sign=" << sign;
		}

		std::ostream request_stream(&request_);

		if("GET" == formatted_method)
		{
			request_stream << formatted_method << " " << url << "?"
				<< param_stream.str() << " HTTP/1.1\r\n";
		}
		else
		{
			request_stream << formatted_method << " " << url << " HTTP/1.1\r\n";
		}

		request_stream << "Host: " << host_ << "\r\n";
		request_stream << "Accept: */*\r\n";

		if("POST" == formatted_method)
		{
			request_stream << "Content-Length: " << param_stream.str().length() << "\r\n";
		}

		request_stream << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
		request_stream << "Connection: close\r\n\r\n";

		if("POST" == formatted_method)
		{
			request_stream << param_stream.str();
		}

		LLOG() << buffer_to_string(request_);
		LLOG();

		boost::asio::async_write(socket_, request_,
			boost::bind(&client::handle_write, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	bool parse_response(Json::Value& root)
	{
		std::istream response_stream(&response_);

		std::string http_version;
		response_stream >> http_version;

		unsigned int status_code;
		response_stream >> status_code;

		std::string status_message;
		std::getline(response_stream, status_message);

		if(!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			LLOG(std::cerr) << "Invalid response";
			return false;
		}

		if(status_code != 200)
		{
			LLOG(std::cerr) << "Response returned with status code " << status_code;
			return false;
		}

		boost::system::error_code ec;

		while(boost::asio::read(socket_, response_, boost::asio::transfer_at_least(1), ec)) {}

		if(ec != boost::asio::error::eof && ec.message() != "short read")
		{
			throw boost::system::system_error(ec);
			return false;
		}

		std::string header;

		while(std::getline(response_stream, header) && header != "\r")
		{
			LLOG() << header;
		}
		LLOG();

		if(0 == response_.size())
		{
			LLOG(std::cerr) << "Empty response";
			return false;
		}

		LLOG() << buffer_to_string(response_);
		LLOG();

		Json::Reader reader;

		if(!reader.parse(response_stream, root))
		{
			LLOG(std::cerr) << "Parse failed";
			return false;
		}

		return true;
	}

	void test_api()
	{
		std::map<std::string, std::string> param;

		// {
		// 	param["symbol"] = "btc_cny";
		// 	send_request("get", "/api/v1/trades.do", param);
		// }

		{
			send_request("post", "/api/v1/userinfo.do", param);
		}
	}

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;

	std::string host_, api_key_, secret_key_;

	bool is_busy_, will_halt_;

	int period_;
	unsigned long round_;
	boost::asio::steady_timer timer_;
};

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		std::string host = "www.okcoin.cn";
		int period = 120;

		client c(service, context, host, period);

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG(std::cerr) << "Exception: " << e.what();
	}

	return 0;
}
