
#ifndef ANALYZER_INCLUDED
#define ANALYZER_INCLUDED

#include "client.h"
#include "command.h"
#include "signals.h"

class analyzer_t
{
public:
	analyzer_t(client_t& client, command_queue_t& queue);
	~analyzer_t();

private:
	client_t& client_;
	command_queue_t& queue_;

	boost::signals2::connection conn_api_handled;

	void on_api_handled(bool status, const api_t& api, const Json::Value& json);
};

#endif /* ANALYZER_INCLUDED */
