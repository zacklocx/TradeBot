
#include <iostream>

#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

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
		std::cout << "Verifying " << subject_name << "\n";

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

			request_stream << "GET /api/v1/ticker.do?symbol=btc_cny HTTP/1.1\r\n";
			request_stream << "Host: www.okcoin.cn\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Connection: close\r\n";
			request_stream << "\r\n";

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
				std::cout << header << "\n";
			}
			std::cout << "\n";

			if(response_.size() > 0)
			{
				std::cout << &response_;
			}

			boost::system::error_code ec;

			while(boost::asio::read(socket_, response_, boost::asio::transfer_at_least(1), ec))
			{
				std::cout << &response_;
			}

			if(ec != boost::asio::error::eof)
			{
				throw boost::system::system_error(ec);
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
