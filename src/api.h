
#ifndef API_INCLUDED
#define API_INCLUDED

#include <map>
#include <string>
#include <iostream>

class api_t
{
public:
	typedef std::map<std::string, std::string> param_type;

	api_t(const std::string& name = "", const std::string& method = "", const param_type& param = param_type()) :
		name_(name), method_(method), param_(param)
	{}

	std::string name() const { return name_; }
	std::string method() const { return method_; }

	param_type param() const { return param_; }

	void update_name(const std::string& name) { name_ = name; }
	void update_method(const std::string& method) { method_ = method; }

	void update_param(const param_type& param) { param_ = param; }
	void update_param(const std::string& key, const std::string& value) { param_[key] = value; }

	std::string url() const { return name_to_url(name_); }

	param_type sign() const;

private:
	std::string name_, method_;
	param_type param_;

	static std::string name_to_url(const std::string& name);
};

std::ostream& operator<<(std::ostream& out, const api_t& api);

#endif /* API_INCLUDED */
