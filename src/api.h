
#ifndef API_INCLUDED
#define API_INCLUDED

#include <map>
#include <string>
#include <iostream>

typedef std::map<std::string, std::string> param_type;

class api_t
{
public:
	api_t(const std::string& host = "", const std::string& interface = "",
		const std::string& method = "", const param_type& param = param_type()) :
		host_(host), interface_(interface), method_(method), param_(param)
	{}

	std::string host() const { return host_; }
	std::string interface() const { return interface_; }
	std::string method() const { return method_; }

	param_type param() const { return param_; }

	void update_host(const std::string& host) { host_ = host; }
	void update_interface(const std::string& interface) { interface_ = interface; }
	void update_method(const std::string& method) { method_ = method; }

	void update_param(const param_type& param) { param_ = param; }
	void update_param(const std::string& key, const std::string& value) { param_[key] = value; }

	std::string prepare() const;

private:
	std::string host_, interface_, method_;
	param_type param_;
};

std::ostream& operator<<(std::ostream& out, const api_t& api);

#endif /* API_INCLUDED */
