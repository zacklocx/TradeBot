
#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>

#include <json/json.h>

#include "TradeBotConfig.h"

#include "dump.h"
#include "utils.h"

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

class client
{
public:
	typedef std::map<std::string, std::string> param_type;

	struct api
	{
		api(const std::string& name, const std::string& method, const param_type& param) :
			name_(name), method_(method), param_(param)
		{}

		std::string name_, method_;
		param_type param_;
	};

	class order
	{
	public:
		order(const std::string& id = "-1",
			const std::string& type = "",
			const std::string& status = "",
			const std::string& price = "0",
			const std::string& amount = "0",
			const std::string& avg_price = "0",
			const std::string& deal_amount = "0") :
			id_(id), type_(type), status_(status),
			price_(price), amount_(amount),
			avg_price_(avg_price), deal_amount_(deal_amount)
		{}

		std::string id() const { return id_; }

		bool valid() const { return std::stoul(id_) > 0; }

		bool buy_type() const { return "buy_market" == type_; }
		bool sell_type() const { return "sell_market" == type_; }

		bool no_deal() const { return "0" == status_; }
		bool part_deal() const { return "1" == status_; }
		bool full_deal() const { return "2" == status_; }

		bool canceling() const { return "4" == status_; }
		bool canceled() const { return "-1" == status_; }

		std::string price() const { return price_; }
		std::string amount() const { return amount_; }

		std::string avg_price() const { return avg_price_; }
		std::string deal_amount() const { return deal_amount_; }

	private:
		std::string id_, type_, status_;
		std::string price_, amount_, avg_price_, deal_amount_;
	};

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

		init_account();
		init_queue();

		is_busy_ = false;
		will_buy_ = will_sell_ = false;
		will_halt_ = (0 == api_queue_.size());

		round_ = 0;
	}

	void init_account()
	{
		money = origin_money = 10000.0;
		btc = origin_btc = 0.0;
		buy_price = sell_price = -1.0;
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
			LLOG(false) << "Timer error: " << ec.message();
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
			LLOG(false) << "Connect failed: " << ec.message();
		}
	}

	void handle_handshake(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			execute_queue();
		}
		else
		{
			LLOG(false) << "Handshake failed: " << ec.message();
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
			LLOG(false) << "Write failed: " << ec.message();
		}
	}

	void handle_read(const boost::system::error_code& ec, size_t bt)
	{
		if(!ec)
		{
			Json::Value data;

			if(parse_response(data))
			{
				std::string name = api_queue_.front().name_;
				dump_json(data, name);

				api_queue_.pop();
				analyze(name, data);

				is_busy_ = false;
				will_halt_ = (0 == api_queue_.size());
			}
		}
		else
		{
			LLOG(false) << "Read failed: " << ec.message();
		}
	}

	void send_request(const std::string& url,
						const std::string& method,
						const param_type& param)
	{
		std::string the_method = method;
		std::transform(the_method.begin(), the_method.end(), the_method.begin(),
			[](char c) { return std::toupper(c); });

		if(the_method != "GET" && the_method != "POST")
		{
			LLOG(false) << "Unsupported method";
			return;
		}

		std::string separator = "";

		std::ostringstream param_stream;

		if("POST" == the_method)
		{
			param_stream << "api_key=" << api_key_ << "&";
		}

		for(const auto& it : param)
		{
			param_stream << separator << it.first << "=" << it.second;
			separator = "&";
		}

		if("POST" == the_method)
		{
			std::ostringstream param_with_key;
			param_with_key << param_stream.str() << separator << "secret_key=" << secret_key_;

			std::string sign = md5(param_with_key.str());

			param_stream << separator << "sign=" << sign;
		}

		std::ostream request_stream(&request_);

		if("GET" == the_method)
		{
			request_stream << the_method << " " << url << "?"
				<< param_stream.str() << " HTTP/1.1\r\n";
		}
		else
		{
			request_stream << the_method << " " << url << " HTTP/1.1\r\n";
		}

		request_stream << "Host: " << host_ << "\r\n";
		request_stream << "Accept: */*\r\n";

		if("POST" == the_method)
		{
			request_stream << "Content-Length: " << param_stream.str().length() << "\r\n";
		}

		request_stream << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
		request_stream << "Connection: close\r\n\r\n";

		if("POST" == the_method)
		{
			request_stream << param_stream.str();
		}

		LLOG() << buffer_to_string(request_);

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
			LLOG(false) << "Invalid response";
			return false;
		}

		if(status_code != 200)
		{
			LLOG(false) << "Response returned with status code " << status_code;
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

		if(0 == response_.size())
		{
			LLOG(false) << "Empty response";
			return false;
		}

		LLOG() << buffer_to_string(response_);

		Json::Reader reader;

		if(!reader.parse(response_stream, root))
		{
			LLOG(false) << "Parse failed";
			return false;
		}

		return true;
	}

	void push_api(const std::string& name, const std::string& method,
		const std::initializer_list<std::pair<std::string const, std::string>>& param_list)
	{
		api_queue_.push(api(name, method, param_type(param_list)));
	}

	void call_api(const api& the_api)
	{
		send_request("/api/v1/" + the_api.name_ + ".do", the_api.method_, the_api.param_);
	}

	void init_queue()
	{
		// api doc: https://www.okcoin.cn/about/rest_getStarted.do

		push_api("userinfo", "post", {});
		push_api("order_info", "post", {{"symbol", "btc_cny"}, {"order_id", "-1"}});
		// push_api("kline", "get", {{"symbol", "btc_cny"}, {"type", "1min"}, {"size", "100"}});
		// push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
		// push_api("trades", "get", {{"symbol", "btc_cny"}});
		// push_api("ticker", "get", {{"symbol", "btc_cny"}});
	}

	void execute_queue()
	{
		if(api_queue_.size() > 0)
		{
			call_api(api_queue_.front());
		}
	}

	void analyze(const std::string& name, const Json::Value& data)
	{
		if("order_info" == name)
		{
			if("false" == data["result"].asString())
			{
				push_api("order_info", "post", {{"symbol", "btc_cny"}, {"order_id", curr_order_.id()}});
				return;
			}

			const Json::Value& orders = data["orders"];

			for(const auto& it : orders)
			{
				order the_order(it["order_id"].asString(), it["type"].asString(), it["status"].asString(),
								it["price"].asString(), it["amount"].asString(),
								it["avg_price"].asString(), it["deal_amount"].asString());

				if(curr_order_.valid())
				{
					if(the_order.id() == curr_order_.id())
					{
						curr_order_ = the_order;

						if(curr_order_.full_deal())
						{
							if(curr_order_.buy_type())
							{
								LLOG(false) << curr_order_.id() << " buy "
									<< curr_order_.avg_price() << " " << curr_order_.deal_amount() << "\n";

								will_sell_ = true;
								push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
							}
							else if(curr_order_.sell_type())
							{
								LLOG(false) << curr_order_.id() << " sell "
									<< curr_order_.avg_price() << " " << curr_order_.deal_amount() << "\n";

								will_buy_ = true;
								push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
							}
						}

						break;
					}
				}
				else
				{
					if(the_order.no_deal() || the_order.part_deal())
					{
						curr_order_ = the_order;
						break;
					}
				}
			}

			if(!curr_order_.valid())
			{
				will_buy_ = true;
				push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
			}
			else
			{
				push_api("order_info", "post", {{"symbol", "btc_cny"}, {"order_id", curr_order_.id()}});
			}

			return;
		}

		if("depth" == name)
		{
			if("false" == data["result"].asString())
			{
				push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
				return;
			}

			if(will_buy_)
			{
				const Json::Value& asks = data["asks"];
				const Json::Value& lowest_ask = asks[asks.size() - 1];

				double buy_price = std::stod(lowest_ask[0].asString());
				double buy_amount = std::stod(lowest_ask[1].asString());

				buy(buy_price, buy_amount);
			}
			else if(will_sell_)
			{
				const Json::Value& bids = data["bids"];
				const Json::Value& highest_bid = bids[0];

				double sell_price = std::stod(highest_bid[0].asString());
				double sell_amount = std::stod(highest_bid[1].asString());

				sell(sell_price, sell_amount);
			}

			return;
		}

		if("trade" == name)
		{
			if("false" == data["result"].asString())
			{
				push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
				return;
			}

			return;
		}
	}

	void dump_account()
	{
		dump_helper_t _("dump_account");

		LLOG(false) << "money: " << money;
		LLOG(false) << "btc: " << btc;
	}

	void buy(double price, double amount)
	{
		if(money > 0.0 && (sell_price < 0.0 || sell_price > price))
		{
			double total = price * amount;

			if(money < total)
			{
				LLOG(false) << "buy " << price << " " << money / price << "\n";

				btc += money / price;
				money = 0.0;
			}
			else
			{
				LLOG(false) << "buy " << price << " " << amount << "\n";

				money -= total;
				btc += amount;
			}

			dump_account();

			will_buy_ = false;
			will_sell_ = true;

			buy_price = price;
		}

		push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
	}

	void sell(double price, double amount)
	{
		if(btc > 0.0 && (buy_price >= 0.0 && buy_price < price))
		{
			if(btc < amount)
			{
				LLOG(false) << "sell " << price << " " << btc << "\n";

				money += price * btc;
				btc = 0;
			}
			else
			{
				LLOG(false) << "sell " << price << " " << amount << "\n";

				money += price * amount;
				btc -= amount;
			}

			dump_account();

			will_buy_ = true;
			will_sell_ = false;

			sell_price = price;
		}

		push_api("depth", "get", {{"symbol", "btc_cny"}, {"size", "200"}, {"merge", "0"}});
	}

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;

	std::string host_, api_key_, secret_key_;

	bool is_busy_, will_buy_, will_sell_, will_halt_;

	int period_;
	unsigned long round_;
	boost::asio::steady_timer timer_;

	std::queue<api> api_queue_;

	order curr_order_;

	double money, btc;
	double origin_money, origin_btc;
	double buy_price, sell_price;
};

int main(int argc, char** argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
		context.set_default_verify_paths();

		std::string host = "www.okcoin.cn";
		int period = 110;

		client the_client(service, context, host, period);

		service.run();
	}
	catch(std::exception& e)
	{
		LLOG(false) << "Exception: " << e.what();
	}

	return 0;
}
