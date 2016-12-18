
#ifndef TICKER_MOD_INCLUDED
#define TICKER_MOD_INCLUDED

#include <vector>

#include "../signals.h"

class ticker_mod_t
{
public:
	ticker_mod_t();
	~ticker_mod_t();

	void init(int capacity, int interval);
	void analyze(float price);

private:
	boost::signals2::connection conn_render;

	int capacity_, interval_;
	std::vector<float> data_;

	float low_, high_;
	float recent_low_, recent_high_;
	std::vector<int> low_points_, high_points_;

	void on_render();
};

#endif
