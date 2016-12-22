
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
	float interval_low_, interval_high_, interval_last_;

	std::vector<int> interval_break_;

	void on_render();
};

#endif
