
#ifndef ANALYZER_INCLUDED
#define ANALYZER_INCLUDED

#include "signals.h"

#include "modules/ticker_mod.h"

class analyzer_t
{
public:
	analyzer_t();
	~analyzer_t();

	void init_modules();

private:
	boost::signals2::connection conn_api_handled;

	ticker_mod_t ticker_mod;

	void on_api_handled(bool status, const api_t& api, const Json::Value& json);
};

#endif /* ANALYZER_INCLUDED */
