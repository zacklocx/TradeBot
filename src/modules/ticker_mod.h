
#ifndef TICKER_MOD_INCLUDED
#define TICKER_MOD_INCLUDED

#include <list>
#include <vector>
#include <string>

#include "../signals.h"

class ticker_mod_t
{
public:
	ticker_mod_t();
	~ticker_mod_t();

	void init(int capacity, int interval);
	void analyze(float price);

private:
	std::string start_time_, current_time_;

	int capacity_, interval_;

	float low_, high_;
	std::vector<float> data_;

	float interval_low_, interval_high_;
	std::vector<int> interval_break_;

	int long_signal_, short_signal_;
	int long_target_, short_target_;
	std::list<int> long_interval_, short_interval_;

	float long_cny_, long_btc_;
	float short_cny_, short_btc_;

	float net_;

	boost::signals2::connection conn_render;

	void on_render();
};

#endif
