
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
	int capacity_, interval_;

	uint64_t start_, now_;

	int delay_, delay_count_, delay_type_;
	float delay_value_;

	float low_, high_;
	std::vector<float> data_;

	float interval_low_, interval_high_;
	std::vector<int> interval_break_, interval_trade_;

	int start_target_, stop_target_;

	int long_signal_, long_target_;
	int short_signal_, short_target_;

	float unit_btc_, max_btc_;

	float long_cny_, long_btc_;
	float short_cny_, short_btc_;

	float profit_;

	boost::signals2::connection conn_render;

	void long_buy(float price);
	void long_sell(float price);

	void short_buy(float price);
	void short_sell(float price);

	void delay_trade(float price);

	void create_buy_api(float cny);
	void create_sell_api(float btc);

	void on_render();
};

#endif
