
#ifndef API_COMMAND_INCLUDED
#define API_COMMAND_INCLUDED

#include "api.h"
#include "client.h"
#include "command.h"

class api_command_t
{
public:
	api_command_t(const api_t& api, client_t& client, client_t::handler_type handler, int priority = 0) :
		api_(api), client_(client), handler_(handler), priority_(priority)
	{}

	int priority() const { return priority_; }
	void execute() { client_.async_call(api_, handler_); }

private:
	api_t api_;

	client_t& client_;
	client_t::handler_type handler_;

	int priority_;
};

inline int priority(const api_command_t& o) { return o.priority(); }
inline void execute(api_command_t& o) { o.execute(); }

#endif /* API_COMMAND_INCLUDED */
