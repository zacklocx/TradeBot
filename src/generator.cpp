
#include "generator.h"

#include <limits>

#include <boost/bind.hpp>

generator_t::generator_t(client_t& client, command_queue_t& queue) : client_(client), queue_(queue)
{
	conn_api_created = sig_api_created.connect(boost::bind(&generator_t::on_api_created, this, _1, _2));
}

generator_t::~generator_t()
{
	conn_api_created.disconnect();
}

void generator_t::generate(const api_t& api, int priority /* = 0 */)
{
	static uint64_t mask = std::numeric_limits<uint64_t>::max() >> 8;
	queue_.push(client_.set(api).set(((int64_t)priority << 56) | mask--));
}

void generator_t::on_api_created(const api_t& api, int priority)
{
	generate(api, priority);
}
