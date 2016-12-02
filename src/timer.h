
#ifndef TIMER_INCLUDED
#define TIMER_INCLUDED

#include <cstdint>

#include <chrono>
#include <utility>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

class timer_t
{
public:
	typedef std::function<void()> handler_type;

	timer_t(boost::asio::io_service& service, int period, handler_type handler) :
		period_(period), count_(0), running_(false),
		handler_(std::move(handler)),
		timer_(service, std::chrono::milliseconds(period))
	{}

	int period() const { return period_; }
	uint64_t count() const { return count_; }
	bool running() const { return running_; }

	void start();
	void stop();
	void reset();

private:
	int period_;
	uint64_t count_;
	bool running_;

	handler_type handler_;

	boost::asio::steady_timer timer_;

	void handle_wait(const boost::system::error_code& ec);
};

#endif /* TIMER_INCLUDED */
