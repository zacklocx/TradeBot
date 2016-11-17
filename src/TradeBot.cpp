#include <cstdlib>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

class client {
public:
    client(boost::asio::io_service& io_service,
        boost::asio::ssl::context& context,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
        : socket_(io_service, context)
    {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback(
            boost::bind(&client::verify_certificate, this, _1, _2));

        boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
            boost::bind(&client::handle_connect, this,
                boost::asio::placeholders::error));
    }

    bool verify_certificate(bool preverified,
        boost::asio::ssl::verify_context& ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // In this example we will simply print the certificate's subject name.
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        std::cout << "Verifying " << subject_name << "\n";

        return preverified;
    }

    void handle_connect(const boost::system::error_code& error)
    {
        if (!error) {
            socket_.async_handshake(boost::asio::ssl::stream_base::client,
                boost::bind(&client::handle_handshake, this,
                    boost::asio::placeholders::error));
        } else {
            std::cout << "Connect failed: " << error.message() << "\n";
        }
    }

    void handle_handshake(const boost::system::error_code& error)
    {
        if (!error) {
            std::ostream request_stream(&request_);

            request_stream << "GET /api/v1/ticker.do?symbol=btc_cny HTTP/1.1\r\n";
            request_stream << "Host: www.okcoin.cn\r\n";
            request_stream << "Accept: */*\r\n";
            request_stream << "Connection: close\r\n";
            request_stream << "\r\n";

            boost::asio::async_write(
                socket_, request_,
                boost::bind(&client::handle_write, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        } else {
            std::cout << "Handshake failed: " << error.message() << "\n";
        }
    }

    void handle_write(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            boost::asio::async_read_until(
                socket_, response_, "\r\n",
                boost::bind(&client::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        } else {
            std::cout << "Write failed: " << error.message() << "\n";
        }
    }

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            std::istream response_stream(&response_);

            std::string http_version;
            response_stream >> http_version;

            unsigned int status_code;
            response_stream >> status_code;

            std::string status_message;

            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                std::cout << "Invalid response\n";
                return;
            }

            if (status_code != 200) {
                std::cout << "Response returned with status code " << status_code
                          << "\n";
                return;
            }

            boost::asio::read_until(socket_, response_, "\r\n\r\n");

            std::string header;

            while (std::getline(response_stream, header) && header != "\r") {
                std::cout << header << "\n";
            }
            std::cout << "\n";

            if (response_.size() > 0) {
                std::cout << &response_;
            }

            boost::system::error_code ec;

            while (boost::asio::read(socket_, response_,
                boost::asio::transfer_at_least(1), ec)) {
                std::cout << &response_;
            }

            if (ec != boost::asio::error::eof) {
                throw boost::system::system_error(ec);
            }
        } else {
            std::cout << "Read failed: " << error.message() << "\n";
        }
    }

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;
};

int main(int argc, char* argv[])
{
    try {
        boost::asio::io_service io_service;

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query("www.okcoin.cn", "https");
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        ctx.set_default_verify_paths();

        client c(io_service, ctx, iterator);

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
