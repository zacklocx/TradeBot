
#ifndef ANALYZER_INCLUDED
#define ANALYZER_INCLUDED

#include "signals.h"

class analyzer_t
{
public:
	analyzer_t();
	~analyzer_t();

private:
	boost::signals2::connection conn_api_handled;

	void on_api_handled(bool status, const api_t& api, const Json::Value& json);
};

#endif /* ANALYZER_INCLUDED */
