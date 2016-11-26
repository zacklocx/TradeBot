
#ifndef ORDER_INCLUDED
#define ORDER_INCLUDED

#include <iostream>

enum class order_type_t
{
	invalid = -1,
	buy,
	sell,
	last
};

enum class order_status_t
{
	invalid = -1,
	no_deal,
	part_deal,
	full_deal,
	canceling,
	canceled,
	last
};

class order_t
{
public:
	order_t(long id = -1,
		order_type_t type = order_type_t::invalid,
		order_status_t status = order_status_t::invalid,
		double price = 0.0,
		double amount = 0.0,
		double avg_price = 0.0,
		double deal_amount = 0.0) :
		id_(id), type_(type), status_(status),
		price_(price), amount_(amount),
		avg_price_(avg_price), deal_amount_(deal_amount)
	{}

	long id() const { return id_; }

	order_type_t type() const { return type_; }
	order_status_t status() const { return status_; }

	double price() const { return price_; }
	double amount() const { return amount_; }

	double avg_price() const { return avg_price_; }
	double deal_amount() const { return deal_amount_; }

	void update_status(order_status_t status) { status_ = status; }

	void update_avg_price(double avg_price) { avg_price_ = avg_price; }
	void update_deal_amount(double deal_amount) { deal_amount_ = deal_amount; }

private:
	long id_;

	order_type_t type_;
	order_status_t status_;

	double price_, amount_;
	double avg_price_, deal_amount_;
};

std::ostream& operator<<(std::ostream& out, const order_t& order);

#endif /* ORDER_INCLUDED */
