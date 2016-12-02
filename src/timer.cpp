
#include "timer.h"

#include <boost/bind.hpp>

void timer_t::start()
{
	if(!running_)
	{
		timer_.async_wait(boost::bind(&timer_t::handle_wait, this, boost::asio::placeholders::error));
		running_ = true;
	}
}

void timer_t::stop()
{
	if(running_)
	{
		timer_.cancel();
		running_ = false;
	}
}

void timer_t::reset()
{
	if(running_)
	{
		timer_.cancel();

		count_ = 0;
		running_ = false;
	}
}

void timer_t::handle_wait(const boost::system::error_code& ec)
{
	if(!ec)
	{
		++count_;

		timer_.expires_at(timer_.expires_at() + std::chrono::milliseconds(period_));
		timer_.async_wait(boost::bind(&timer_t::handle_wait, this, boost::asio::placeholders::error));

		if(handler_)
		{
			handler_();
		}
	}
}
