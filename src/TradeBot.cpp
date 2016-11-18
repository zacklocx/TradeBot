
#include <string>
#include <sstream>
#include <iostream>

#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <openssl/md5.h>
#include <json/json.h>

std::string md5(const std::string &s)
{
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)s.c_str(), s.length(), digest);

	char result[MD5_DIGEST_LENGTH * 2 + 1];

	for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(result + i * 2, "%02X", digest[i]);
	}

	return result;
}

std::string urlencode(const std::string &s)
{
	const char lookup[]= "0123456789abcdef";

	std::ostringstream result;

	for(int i = 0, len = s.length(); i < len; ++i)
	{
		const char& c = s[i];

		if((48 <= c && c <= 57) || // 0-9
			(65 <= c && c <= 90) || // a-z
			(97 <= c && c <= 122) || // A-Z
			(c == '-' || c == '_' || c == '.' || c == '~'))
		{
			result << c;
		}
		else
		{
			result << '%';
			result << lookup[(c & 0xF0) >> 4];
			result << lookup[(c & 0x0F)];
		}
	}

	return result.str();
}

void dump_param(const std::map<std::string, std::string>& m)
{
	for(const auto& it : m)
	{
		std::cout << it.first << " = " << it.second << "\n";
	}
}

void dump_value(const Json::Value& v)
{
	std::cout << v.toStyledString() << "\n";
}

struct dump_helper
{
	std::string msg_;

	dump_helper(const std::string& msg = "") : msg_(msg)
	{
		std::cout << "------------ " << msg_ << " begin ------------\n";
	}

	~dump_helper()
	{
		std::cout << "------------ " << msg_ << " end ------------\n\n";
	}
};

class client
{
public:
	client(boost::asio::io_service& service,
			boost::asio::ssl::context& context,
			const std::string& host)
			: socket_(service, context), host_(host)
	{
		init_info();

		socket_.set_verify_mode(boost::asio::ssl::verify_peer);
		socket_.set_verify_callback(boost::bind(&client::verify_certificate, this, _1, _2));

		boost::asio::ip::tcp::resolver resolver(service);
		boost::asio::ip::tcp::resolver::query query(host_, "https");
		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

		boost::asio::async_connect(socket_.lowest_layer(), iterator,
			boost::bind(&client::handle_connect, this, boost::asio::placeholders::error));
	}

	bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& context)
	{
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
		// std::cout << "Verifying " << subject_name << "\n";

		return preverified;
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
			std::cout << "Connect failed: " << ec.message() << "\n";
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
			std::cout << "Handshake failed: " << ec.message() << "\n";
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
			std::cout << "Write failed: " << ec.message() << "\n";
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
			}
		}
		else
		{
			std::cout << "Read failed: " << ec.message() << "\n";
		}
	}

	void init_info()
	{
		api_key_ = "91a440cb-d0c2-4dd6-924e-320d2a95a543";
		secret_key_ = "8AF27D70C7D51568ADC74FA0CBF707F0";
	}

	void send_request(const std::string& method,
						const std::string& url,
						std::map<std::string, std::string>& param)
	{
		std::string formatted_method(method);
		std::transform(formatted_method.begin(), formatted_method.end(), formatted_method.begin(),
			[](char c) { return std::toupper(c); });

		std::cout << formatted_method << " " << url << "\n";

		{
			dump_helper _("dump param");
			dump_param(param);
		}

		if(formatted_method != "GET" && formatted_method != "POST")
		{
			std::cout << "Unsupported method\n";
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
		// request_stream << "Connection: keep-alive\r\n\r\n";

		if("POST" == formatted_method)
		{
			request_stream << param_stream.str();
		}

		// std::cout << &request_ << "\n";

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
			std::cout << "Invalid response\n";
			return false;
		}

		if(status_code != 200)
		{
			std::cout << "Response returned with status code " << status_code << "\n";
			return false;
		}

		boost::asio::read_until(socket_, response_, "\r\n\r\n");

		std::string header;

		while(std::getline(response_stream, header) && header != "\r")
		{
			// std::cout << header << "\n";
		}
		// std::cout << "\n";

		if(0 == response_.size())
		{
			std::cout << "Empty response\n";
			return false;
		}

		// std::cout << &response_;

		boost::system::error_code ec;

		while(boost::asio::read(socket_, response_, boost::asio::transfer_at_least(1), ec))
		{
			// std::cout << &response;
		}

		if(ec != boost::asio::error::eof && ec.message() != "short read")
		{
			throw boost::system::system_error(ec);
			return false;
		}

		Json::Reader reader;

		if(!reader.parse(response_stream, root))
		{
			std::cout << "Parse failed\n";
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
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;

	std::string host_, api_key_, secret_key_;
};

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		std::string host = "www.okcoin.cn";

		client c(service, context, host);

		service.run();
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
