
#include "ticker_mod.h"

#include <cmath>

#include <algorithm>

#include <boost/bind.hpp>

#include <GL/freeglut.h>

#include "imgui.h"

#include "../dump.h"
#include "../renderer.h"

ticker_mod_t::ticker_mod_t() :
	capacity_(0), interval_(0),
	low_(0.0f), high_(0.0f),
	interval_low_(0.0f), interval_high_(0.0f), interval_last_(0.0f)
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
	if(!(interval_ > 0 && capacity_ >= interval_))
	{
		return;
	}

	size_t size = data_.size();

	if(size >= capacity_ || 0 == size)
	{
		data_.clear();
		size = 0;

		low_ = high_ = price;
		interval_low_ = interval_high_ = interval_last_ = 0.0f;

		interval_break_.clear();
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

	if(size > interval_)
	{
		auto end = data_.end() - 1;
		auto begin = end - interval_;

		interval_low_ = *std::min_element(begin, end);
		interval_high_ = *std::max_element(begin, end);

		if(price < interval_low_)
		{
			interval_low_ = interval_last_ = price;
			interval_break_.push_back(1 - size);
		}
		else if(price > interval_high_)
		{
			interval_high_ = interval_last_ = price;
			interval_break_.push_back(size - 1);
		}
	}
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

	if((mouse_x - padding >= 0 && mouse_x - padding < content_width) &&
		(mouse_y - padding >= 0 && mouse_y - padding < content_height))
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
		float x = 1.0f * content_width / division_x * i + 1.0f;

		glVertex2f(x, -1.0f);
		glVertex2f(x, (i % 2)? -11.0f : -21.0f);
	}

	for(int n = std::floor(low_ / unit_price), i = 0; i <= division_y; ++i)
	{
		float y = (unit_price * (n + i) - low_ + unit_price) * scale_y;

		glVertex2f(content_width + 1.0f, y);
		glVertex2f(content_width + (((n + i) % _1_unit_price)? 11.0f : 21.0f), y);
	}

	glColor3ub(255, 0, 0);

	for(size_t i = 0; i < size; ++i)
	{
		float x = i * scale_x + 1.0f;

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

			glVertex2f(it * scale_x, (data_[it] - low_ + unit_price) * scale_y);
		}

		glEnd();

		glBegin(GL_LINES);

		for(auto it : interval_break_)
		{
			if(it < 0)
			{
				it = -it;
				glColor3ub(255, 255, 0);
			}
			else
			{
				glColor3ub(0, 255, 0);
			}

			float x = it * scale_x + 1.0f;
			float y = (data_[it] - low_ + unit_price) * scale_y;

			for(int i = -2; i <= 2; ++i)
			{
				glVertex2f(x + i, y);
				glVertex2f(x + i, y - 5.0f);
			}
		}
	}

	glEnd();

	glPopMatrix();

	ImGui::Text("low:"); ImGui::SameLine(80); ImGui::Text("%f", low_);
	ImGui::Text("high:"); ImGui::SameLine(80); ImGui::Text("%f", high_);
	ImGui::Text("last:"); ImGui::SameLine(80); ImGui::Text("%f", last_price);

	ImGui::Separator();

	ImGui::Text("i_low:"); ImGui::SameLine(80); ImGui::Text("%f", interval_low_);
	ImGui::Text("i_high:"); ImGui::SameLine(80); ImGui::Text("%f", interval_high_);
	ImGui::Text("i_last:"); ImGui::SameLine(80); ImGui::Text("%f", interval_last_);

	ImGui::Separator();

	ImGui::Text("selected:"); ImGui::SameLine(80); ImGui::Text("%f", selected_price);
}
