
#ifndef API_INCLUDED
#define API_INCLUDED

#include <map>
#include <string>
#include <iostream>

class api_t
{
public:
	typedef std::map<std::string, std::string> param_type;

	api_t(const std::string& url = "", const std::string& method = "", const param_type& param = param_type()) :
		url_(url), method_(method), param_(param)
	{}

	std::string url() const { return url_; }
	std::string method() const { return method_; }

	param_type param() const { return param_; }

	void update_url(const std::string& url) { url_ = url; }
	void update_method(const std::string& method) { method_ = method; }

	void update_param(const param_type& param) { param_ = param; }
	void update_param(const std::string& key, const std::string& value) { param_[key] = value; }

	param_type prepare() const;

private:
	std::string url_, method_;
	param_type param_;
};

std::ostream& operator<<(std::ostream& out, const api_t& api);

#endif /* API_INCLUDED */
