
#ifndef TICKER_MOD_INCLUDED
#define TICKER_MOD_INCLUDED

#include <cstdint>

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
	uint64_t start_, now_;

	int capacity_, interval_;

	float low_, high_;
	std::vector<float> data_;

	float interval_low_, interval_high_;
	std::vector<int> interval_break_;

	int long_signal_, long_target_;
	int short_signal_, short_target_;

	float long_cny_, long_btc_;
	float short_cny_, short_btc_;

	float net_profit_;

	boost::signals2::connection conn_render;

	void on_render();
};

#endif
