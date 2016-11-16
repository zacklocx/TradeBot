
#include "TradeBotConfig.h"

#include <string>
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char** argv)
{
	try
	{
		std::cout << "TradeBot Start\n";

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query("www.boost.org", "http");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		request_stream << "GET /LICENSE_1_0.txt HTTP/1.0\r\n";
		request_stream << "Host: www.boost.org\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n"; 

		boost::asio::write(socket, request);

		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		std::istream response_stream(&response);

		std::string http_version;
		response_stream >> http_version;

		unsigned int status_code;
		response_stream >> status_code;

		std::string status_message;

		std::getline(response_stream, status_message);
		if(!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "Invalid response\n";
			return 1;
		}

		if(status_code != 200)
		{
			std::cout << "Response returned with status code " << status_code << "\n";
			return 1;
		}

		boost::asio::read_until(socket, response, "\r\n\r\n");

		std::string header;

		while(std::getline(response_stream, header) && header != "\r")
		{
			std::cout << header << "\n";
		}
		std::cout << "\n";

		if(response.size() > 0)
		{
			std::cout << &response;
		}

		boost::system::error_code ec;

		while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec))
		{
			std::cout << &response;
		}

		if(ec != boost::asio::error::eof)
		{
			throw boost::system::system_error(ec);
		}		
	}
	catch(std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}
