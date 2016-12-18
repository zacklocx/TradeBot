
#include "ticker_mod.h"

#include <iterator>
#include <algorithm>

#include <boost/bind.hpp>

#include <GL/freeglut.h>

#include "imgui.h"

#include "../renderer.h"

ticker_mod_t::ticker_mod_t() :
	capacity_(0), interval_(0),
	low_(0.0f), high_(0.0f),
	recent_low_(0.0f), recent_high_(0.0f)
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
	if(0 == capacity_ || 0 == interval_)
	{
		return;
	}

	size_t size = data_.size();

	if(size >= capacity_)
	{
		data_.clear();
		size = 0;

		low_ = high_ = 0.0f;
		recent_low_ = recent_high_ = 0.0f;

		low_points_.clear();
		high_points_.clear();
	}

	data_.push_back(price);
	++size;

	if(1 == size)
	{
		low_ = high_ = price;
	}
	else
	{
		if(price < low_)
		{
			low_ = price;
		}
		else if(price > high_)
		{
			high_ = price;
		}
	}

	if(size > interval_)
	{
		auto begin = std::begin(data_) + size - 1 - interval_;
		auto end = std::begin(data_) + size - 1;

		recent_low_ = *std::min_element(begin, end);
		recent_high_ = *std::max_element(begin, end);

		if(price < recent_low_)
		{
			recent_low_ = price;
			low_points_.push_back(size - 1);
		}
		else if(price > recent_high_)
		{
			recent_high_ = price;
			high_points_.push_back(size - 1);
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
	mouse_state_t mouse_state = renderer_t::mouse_state();

	int padding = 100;

	int content_width = window_width - 2 * padding;
	int content_height = window_height - 2 * padding;

	float scale_x = 1.0f * content_width / capacity_;
	float scale_y = content_height / (high_ - low_ + 1.0f);

	int division_x = content_width / interval_ * 2;
	int division_y = 0;

	float last_price = data_[size - 1];

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(padding, padding, 0.0f);

	glBegin(GL_LINES);

	glColor3ub(255, 0, 0);

	for(size_t i = 0; i < size; ++i)
	{
		glVertex2f(i * scale_x, 0.0f);
		glVertex2f(i * scale_x, (data_[i] - low_ + 1.0f) * scale_y);
	}

	int selected = ((mouse_state.x_ - padding) / scale_x + 0.5f);
	float selected_price = 0.0f;

	if(selected >= 0 && selected < size)
	{
		selected_price = data_[selected];

		glColor3ub(0, 255, 255);

		glVertex2f(selected * scale_x, 0.0f);
		glVertex2f(selected * scale_x, (selected_price - low_ + 1.0f) * scale_y);
	}

	glColor3ub(0, 0, 255);

	glVertex2f(-1.0f, 0.0f);
	glVertex2f(content_width + 1.0f, 0.0f);

	glVertex2f(-1.0f, content_height);
	glVertex2f(content_width + 1.0f, content_height);

	glVertex2f(-1.0f, 0.0f);
	glVertex2f(-1.0f, content_height);

	glVertex2f(content_width + 1.0f, 0.0f);
	glVertex2f(content_width + 1.0f, content_height);

	for(int i = 0; i <= division_x; ++i)
	{
		glVertex2f(i * content_width / division_x, 0.0f);
		glVertex2f(i * content_width / division_x, (i & 1)? -10.0f : -20.0f);
	}

	glColor3ub(255, 255, 255);

	glVertex2f(-1.0f, (last_price - low_ + 1.0f) * scale_y);
	glVertex2f(content_width, (last_price - low_ + 1.0f) * scale_y);

	glEnd();

	if(size > interval_)
	{
		glPushAttrib(GL_POINT_BIT);
		glPointSize(3.0f);

		glBegin(GL_POINTS);

		glColor3ub(255, 255, 0);

		for(const auto& index : low_points_)
		{
			glVertex2f(index * scale_x - 1.0f, (data_[index] - low_ + 1.0f) * scale_y + 1.0f);
		}

		glColor3ub(0, 255, 0);

		for(const auto& index : high_points_)
		{
			glVertex2f(index * scale_x - 1.0f, (data_[index] - low_ + 1.0f) * scale_y + 1.0f);
		}

		glEnd();

		glPopAttrib();
	}

	ImGui::Text("low: %f", low_);
	ImGui::Text("high: %f", high_);
	ImGui::Text("last: %f", last_price);

	if(selected_price > 0.0f)
	{
		ImGui::Text("selected: %f", selected_price);
	}
}
