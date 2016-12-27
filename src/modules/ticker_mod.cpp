
#include "ticker_mod.h"

#include <cmath>

#include <string>
#include <algorithm>
#include <initializer_list>

#include <boost/bind.hpp>

#include <GL/freeglut.h>

#include "imgui.h"

#include "../dump.h"
#include "../utils.h"
#include "../renderer.h"

ticker_mod_t::ticker_mod_t() :
	capacity_(0), interval_(0),
	start_(timestamp_s()), now_(start_),
	delay_(5), delay_count_(0), delay_type_(0), delay_value_(0.0f),
	low_(0.0f), high_(0.0f),
	interval_low_(0.0f), interval_high_(0.0f),
	start_target_(2), stop_target_(2),
	long_signal_(0), long_target_(5),
	short_signal_(0), short_target_(5),
	unit_btc_(1.0f), max_btc_(2.0f),
	long_cny_(0.0f), long_btc_(0.0f), short_cny_(0.0f), short_btc_(0.0f),
	profit_(0.0f)
{
	conn_render = sig_render.connect(boost::bind(&ticker_mod_t::on_render, this));
}

ticker_mod_t::~ticker_mod_t()
{
	conn_render.disconnect();
}

void ticker_mod_t::init(int capacity, int interval)
{
	capacity_ = capacity;
	interval_ = interval;
}

void ticker_mod_t::analyze(float price)
{
	now_ = timestamp_s();

	if(!(interval_ > 0 && capacity_ >= interval_))
	{
		return;
	}

	size_t size = data_.size();

	if(size >= capacity_ || 0 == size)
	{
		low_ = high_ = price;
		data_.clear();
		size = 0;

		interval_low_ = interval_high_ = 0.0f;

		interval_break_.clear();
		interval_trade_.clear();
	}

	data_.push_back(price);
	++size;

	if(price < low_)
	{
		low_ = price;
	}
	else if(price > high_)
	{
		high_ = price;
	}

	if(delay_type_ != 0)
	{
		if(++delay_count_ >= delay_)
		{
			delay_trade(price);
		}
	}

	if(size > interval_)
	{
		auto end = data_.end() - 1;
		auto begin = end - interval_;

		interval_low_ = *std::min_element(begin, end);
		interval_high_ = *std::max_element(begin, end);

		bool check_signal = false;

		if(price < interval_low_)
		{
			interval_low_ = price;
			interval_break_.push_back(-size);

			long_signal_ = (long_signal_ > 0)? -1 : long_signal_ - 1;
			short_signal_ = (short_signal_ > 0)? -1 : short_signal_ - 1;

			check_signal = true;
		}
		else if(price > interval_high_)
		{
			interval_high_ = price;
			interval_break_.push_back(size);

			long_signal_ = (long_signal_ < 0)? 1 : long_signal_ + 1;
			short_signal_ = (short_signal_ < 0)? 1 : short_signal_ + 1;

			check_signal = true;
		}

		if(check_signal && 0 == delay_type_)
		{
			if(long_signal_ <= -stop_target_ || long_signal_ >= long_target_)
			{
				long_signal_ = 0;
				long_sell(price);
			}
			else if(long_signal_ >= start_target_)
			{
				long_buy(price);
			}

			if(short_signal_ >= stop_target_ || short_signal_ <= -short_target_)
			{
				short_signal_ = 0;
				short_buy(price);
			}
			else if(short_signal_ <= -start_target_)
			{
				short_sell(price);
			}

			profit_ = long_cny_ + short_cny_ + (long_btc_ + short_btc_) * price;
		}
	}
}

void ticker_mod_t::long_buy(float price)
{
	if(long_btc_ + unit_btc_ <= max_btc_)
	{
		float cny = unit_btc_ * price;

		if(0 == delay_)
		{
			long_cny_ -= cny;
			long_btc_ += unit_btc_;

			interval_trade_.push_back(data_.size());
		}
		else if(0 == delay_type_)
		{
			delay_count_ = 0;
			delay_type_ = 1;
			delay_value_ = cny;
		}
	}
}

void ticker_mod_t::long_sell(float price)
{
	if(long_btc_ > 0.0)
	{
		float btc = long_btc_;

		if(0 == delay_)
		{
			long_cny_ += btc * price;
			long_btc_ -= btc;

			interval_trade_.push_back(-data_.size());
		}
		else if(0 == delay_type_)
		{
			delay_count_ = 0;
			delay_type_ = -1;
			delay_value_ = btc;
		}
	}
}

void ticker_mod_t::short_buy(float price)
{
	if(short_cny_ > 0.0)
	{
		float cny = short_cny_;

		if(0 == delay_)
		{
			short_cny_ -= cny;
			short_btc_ += cny / price;

			interval_trade_.push_back(data_.size());
		}
		else if(0 == delay_type_)
		{
			delay_count_ = 0;
			delay_type_ = 2;
			delay_value_ = cny;
		}
	}
}

void ticker_mod_t::short_sell(float price)
{
	if(short_cny_ <= (max_btc_ - unit_btc_) * price)
	{
		float btc = unit_btc_;

		if(0 == delay_)
		{
			short_cny_ += btc * price;
			short_btc_ -= btc;

			interval_trade_.push_back(-data_.size());
		}
		else if(0 == delay_type_)
		{
			delay_count_ = 0;
			delay_type_ = -2;
			delay_value_ = btc;
		}
	}
}

void ticker_mod_t::delay_trade(float price)
{
	float cny_change = 0.0f, btc_change = 0.0f;

	size_t size = data_.size();

	if(delay_type_ > 0)
	{
		cny_change = -delay_value_;
		btc_change = delay_value_ / price;

		interval_trade_.push_back(size);
	}
	else if(delay_type_ < 0)
	{
		cny_change = delay_value_ * price;
		btc_change = -delay_value_;

		interval_trade_.push_back(-size);
	}

	if(1 == delay_type_ || -1 == delay_type_)
	{
		long_cny_ += cny_change;
		long_btc_ += btc_change;
	}
	else if(2 == delay_type_ || -2 == delay_type_)
	{
		short_cny_ += cny_change;
		short_btc_ += btc_change;
	}

	delay_count_ = 0;
	delay_type_ = 0;
	delay_value_ = 0.0f;
}

void ticker_mod_t::create_buy_api(float cny)
{
	api_t trade_api("trade", "POST", {{"symbol", "btc_cny"}, {"type", "buy_market"}, {"price", std::to_string(cny)}});
	// sig_api_created(trade_api, 1);
}

void ticker_mod_t::create_sell_api(float btc)
{
	api_t trade_api("trade", "POST", {{"symbol", "btc_cny"}, {"type", "sell_market"}, {"amount", std::to_string(btc)}});
	// sig_api_created(trade_api, 1);
}

void ticker_mod_t::on_render()
{
	size_t size = data_.size();

	if(0 == size)
	{
		return;
	}

	int window_width = renderer_t::window_width();
	int window_height = renderer_t::window_height();

	int mouse_x = renderer_t::mouse_x();
	int mouse_y = renderer_t::mouse_y();

	int padding = 100;

	int content_width = window_width - padding * 2;
	int content_height = window_height - padding * 2;

	float unit_price = 0.1f;
	int _1_unit_price = 1.0f / unit_price;

	float scale_x = 1.0f * content_width / capacity_;
	float scale_y = content_height / (high_ - low_ + unit_price);

	int division_x = content_width / (interval_ * scale_x) * 2;
	int division_y = std::floor(high_ / unit_price) - std::floor(low_ / unit_price);

	float last_price = data_[size - 1];

	int selected = -1;
	float selected_price = 0.0f;

	if(mouse_y - padding >= 0 && mouse_y - padding < content_height)
	{
		selected = (mouse_x - padding) / scale_x;

		if(selected >= 0 && selected < size)
		{
			selected_price = data_[selected];
		}
	}

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(padding, padding, 0.0f);

	glBegin(GL_LINES);

	glColor3ub(0, 0, 255);

	glVertex2f(-1.0f, 0.0f);
	glVertex2f(content_width + 1.0f, 0.0f);

	glVertex2f(-1.0f, content_height + 1.0f);
	glVertex2f(content_width + 1.0f, content_height + 1.0f);

	glVertex2f(0.0f, -1.0f);
	glVertex2f(0.0f, content_height + 1.0f);

	glVertex2f(content_width + 1.0f, -1.0f);
	glVertex2f(content_width + 1.0f, content_height + 1.0f);

	glColor3ub(255, 255, 255);

	for(int i = 0; i <= division_x; ++i)
	{
		float x = 0.5f * interval_ * i * scale_x + 1.0f;

		glVertex2f(x, -1.0f);
		glVertex2f(x, (i % 2)? -11.0f : -21.0f);
	}

	for(int n = std::floor(low_ / unit_price), i = 0; i <= division_y; ++i)
	{
		float y = (unit_price * (n + i) - low_ + unit_price) * scale_y;

		glVertex2f(content_width + 1.0f, y);
		glVertex2f(content_width + (((n + i) % _1_unit_price)? 11.0f : 21.0f), y);
	}

	for(size_t i = 0; i < size; ++i)
	{
		float x = i * scale_x + 1.0f;

		glColor3ub(255, 0, 0);

		for(auto it : interval_trade_)
		{
			if(it == (i + 1) || it == -(i + 1))
			{
				glColor3ub((it > 0)? 0 : 255, 255, 0);
				break;
			}
		}

		glVertex2f(x, 0.0f);
		glVertex2f(x, (data_[i] - low_ + unit_price) * scale_y);
	}

	if(selected_price > 0.0f)
	{
		glColor3ub(0, 255, 255);

		float x = selected * scale_x + 1.0f;

		glVertex2f(x, 0.0f);
		glVertex2f(x, (selected_price - low_ + unit_price) * scale_y);
	}

	if(size > interval_)
	{
		glEnd();

		glBegin(GL_LINE_STRIP);

		glColor3ub(255, 255, 255);

		for(auto it : interval_break_)
		{
			if(it < 0)
			{
				it = -it;
			}

			glVertex2f((it - 1) * scale_x, (data_[it - 1] - low_ + unit_price) * scale_y);
		}

		glEnd();

		glBegin(GL_LINES);

		for(auto it : interval_break_)
		{
			glColor3ub((it > 0)? 0 : 255, 255, 0);

			if(it < 0)
			{
				it = -it;
			}

			float x = (it - 1) * scale_x + 1.0f;
			float y = (data_[it - 1] - low_ + unit_price) * scale_y;

			for(int i = -2; i <= 2; ++i)
			{
				glVertex2f(x + i, y);
				glVertex2f(x + i, y - 5.0f);
			}
		}
	}

	glEnd();

	glPopMatrix();

	int pos = 120;

	if(ImGui::TreeNode("time"))
	{
		ImGui::Text("start"); ImGui::SameLine(pos); ImGui::Text("%s", what_time(start_).c_str());
		ImGui::Text("now"); ImGui::SameLine(pos); ImGui::Text("%s", now().c_str());

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("delay"))
	{
		ImGui::Text("delay"); ImGui::SameLine(pos); ImGui::Text("%d", delay_);
		ImGui::Text("delay_count"); ImGui::SameLine(pos); ImGui::Text("%d", delay_count_);
		ImGui::Text("delay_type"); ImGui::SameLine(pos); ImGui::Text("%d", delay_type_);

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("price"))
	{
		ImGui::Text("low"); ImGui::SameLine(pos); ImGui::Text("%f", low_);
		ImGui::Text("high"); ImGui::SameLine(pos); ImGui::Text("%f", high_);
		ImGui::Text("last"); ImGui::SameLine(pos); ImGui::Text("%f", last_price);

		ImGui::Separator();

		ImGui::Text("i_low"); ImGui::SameLine(pos); ImGui::Text("%f", interval_low_);
		ImGui::Text("i_high"); ImGui::SameLine(pos); ImGui::Text("%f", interval_high_);

		ImGui::Separator();

		ImGui::Text("selected"); ImGui::SameLine(pos); ImGui::Text("%f", selected_price);

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("signal"))
	{
		ImGui::Text("l_signal"); ImGui::SameLine(pos); ImGui::Text("%d", long_signal_);
		ImGui::Text("l_target"); ImGui::SameLine(pos); ImGui::Text("%d", long_target_);

		ImGui::Separator();

		ImGui::Text("s_signal"); ImGui::SameLine(pos); ImGui::Text("%d", short_signal_);
		ImGui::Text("s_target"); ImGui::SameLine(pos); ImGui::Text("%d", -short_target_);

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("account"))
	{
		ImGui::Text("unit_btc"); ImGui::SameLine(pos); ImGui::Text("%f", unit_btc_);
		ImGui::Text("max_btc"); ImGui::SameLine(pos); ImGui::Text("%f", max_btc_);

		ImGui::Separator();

		ImGui::Text("l_cny"); ImGui::SameLine(pos); ImGui::Text("%f", long_cny_);
		ImGui::Text("l_btc"); ImGui::SameLine(pos); ImGui::Text("%f", long_btc_);

		ImGui::Separator();

		ImGui::Text("s_cny"); ImGui::SameLine(pos); ImGui::Text("%f", short_cny_);
		ImGui::Text("s_btc"); ImGui::SameLine(pos); ImGui::Text("%f", short_btc_);

		ImGui::TreePop();
	}

	if(ImGui::TreeNode("profit"))
	{
		ImGui::Text("profit"); ImGui::SameLine(pos); ImGui::Text("%f", profit_);
		ImGui::Text("profit/min"); ImGui::SameLine(pos); ImGui::Text("%f", 60.0f * profit_ / (now_ - start_));

		ImGui::TreePop();
	}
}
