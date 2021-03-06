
#include "callback_timer.h"

#include <boost/bind.hpp>

void callback_timer_t::start()
{
	if(!running_)
	{
		running_ = true;
		timer_->async_wait(boost::bind(&callback_timer_t::handle_wait, this, boost::asio::placeholders::error));
	}
}

void callback_timer_t::stop()
{
	if(running_)
	{
		running_ = false;
		timer_->cancel();
	}
}

void callback_timer_t::reset()
{
	if(running_)
	{
		count_ = 0;
		running_ = false;

		timer_->cancel();
	}
}

void callback_timer_t::handle_wait(const boost::system::error_code& ec)
{
	if(!ec)
	{
		++count_;

		timer_->expires_at(timer_->expires_at() + std::chrono::milliseconds(period_));
		timer_->async_wait(boost::bind(&callback_timer_t::handle_wait, this, boost::asio::placeholders::error));

		if(handler_)
		{
			handler_();
		}
	}
}
