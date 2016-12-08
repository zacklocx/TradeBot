
#ifndef GENERATOR_INCLUDED
#define GENERATOR_INCLUDED

#include "api.h"
#include "client.h"
#include "command.h"
#include "signals.h"

class generator_t
{
public:
	generator_t(client_t& client, command_queue_t& queue);
	~generator_t();

	void generate(const api_t& api, int priority = 0);

private:
	client_t& client_;
	command_queue_t& queue_;

	boost::signals2::connection conn_api_created;

	void on_api_created(const api_t& api, int priority);
};

#endif /* GENERATOR_INCLUDED */
