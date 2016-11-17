
#include <string>
#include <iostream>

#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <openssl/md5.h>
#include <json/json.h>

std::string urlencode(const std::string &s)
{
	static const char lookup[]= "0123456789abcdef";

	std::stringstream e;

	for(int i = 0, len = s.length(); i < len; ++i)
	{
		const char& c = s[i];

		if((48 <= c && c <= 57) || // 0-9
			(65 <= c && c <= 90) || // a-z
			(97 <= c && c <= 122) || // A-Z
			(c == '-' || c == '_' || c == '.' || c == '~'))
		{
			e << c;
		}
		else
		{
			e << '%';
			e << lookup[(c & 0xF0) >> 4];
			e << lookup[(c & 0x0F)];
		}
	}

	return e.str();
}

class client
{
public:
	client(boost::asio::io_service& service,
			boost::asio::ssl::context& context,
			boost::asio::ip::tcp::resolver::iterator endpoint)
			: socket_(service, context)
	{
		socket_.set_verify_mode(boost::asio::ssl::verify_peer);
		socket_.set_verify_callback(boost::bind(&client::verify_certificate, this, _1, _2));

		boost::asio::async_connect(socket_.lowest_layer(), endpoint,
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
			std::ostream request_stream(&request_);

			std::string post_param = "api_key=91a440cb-d0c2-4dd6-924e-320d2a95a543";
			std::string post_param_with_key = post_param + "&secret_key=8AF27D70C7D51568ADC74FA0CBF707F0";

			unsigned char md5_result[MD5_DIGEST_LENGTH];
			MD5((unsigned char*)post_param_with_key.c_str(), post_param_with_key.length(), md5_result);

			char sign[MD5_DIGEST_LENGTH * 2 + 1];

			for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
			{
				sprintf(sign + i * 2, "%02X", md5_result[i]);
			}

			// urlencode编码的是参数值里面的特殊字符，而不是对整个链接编码，如果'&'和'='被编码，服务器将无法正确识别参数
			// post_param = urlencode(post_param + "&sign=" + sign);

			post_param = post_param + "&sign=" + sign;

			request_stream << "POST /api/v1/userinfo.do HTTP/1.1\r\n";
			request_stream << "Host: www.okcoin.cn\r\n";
			request_stream << "Accept: */*\r\n";			
			request_stream << "Content-Length: " << post_param.length() << "\r\n";
			request_stream << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
			request_stream << "Connection: close\r\n";
			request_stream << "\r\n";
			request_stream << post_param;

			// std::cout << &request_ << "\n";

			boost::asio::async_write(socket_, request_,
				boost::bind(&client::handle_write, this,
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
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
				return;
			}

			if(status_code != 200)
			{
				std::cout << "Response returned with status code " << status_code << "\n";
				return;
			}

			boost::asio::read_until(socket_, response_, "\r\n\r\n");

			std::string header;

			while(std::getline(response_stream, header) && header != "\r")
			{
				// std::cout << header << "\n";
			}
			// std::cout << "\n";

			if(response_.size() > 0)
			{
				// std::cout << &response_;

				Json::Value root;
				Json::Reader reader;

				bool parse_ok = reader.parse(response_stream, root);

				if(!parse_ok)
				{
					std::cout << "Parse failed\n";
				}
				else
				{
					double net = std::stod(root["info"]["funds"]["asset"]["net"].asString());
					std::cout << "net: " << net << "\n";
				}
			}
		}
		else
		{
			std::cout << "Read failed: " << ec.message() << "\n";
		}
	}

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
};

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ip::tcp::resolver resolver(service);
		boost::asio::ip::tcp::resolver::query query("www.okcoin.cn", "https");
		boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(query);

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		client c(service, context, endpoint);

		service.run();
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
