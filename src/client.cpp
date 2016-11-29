
#include "client.h"

#include <string>
#include <iostream>

#include "dump.h"

void client_t::call(const api_t& api, boost::system::error_code& ec, Json::Value& data)
{
	std::string method = api.method();

	if("GET" == method)
	{
		std::string query = avhttp::map_to_query(api.param());
		std::string full_url = api.url() + "?" + query;

		stream_.open(full_url, ec);
	}
	else if("POST" == method)
	{
		avhttp::post_form(stream_, api.prepare());
		stream_.open(api.url(), ec);
	}

	if(!ec)
	{
		std::istream in(&stream_);

		std::string s, body;

		while(in >> s)
		{
			body.append(s);
		}

		Json::Reader reader;
		reader.parse(body, data);
	}
	else
	{
		LLOG() << "error: " << ec.message();
	}

	stream_.close();
}
