
#include "order.h"

#include <map>
#include <string>

std::ostream& operator<<(std::ostream& out, const order_t& order)
{
	static std::map<order_type_t, std::string> type_map;
	static std::map<order_status_t, std::string> status_map;

	if(0 == type_map.size())
	{
		type_map[order_type_t::invalid] = "invalid";
		type_map[order_type_t::buy] = "buy";
		type_map[order_type_t::sell] = "sell";
	}

	if(0 == status_map.size())
	{
		status_map[order_status_t::invalid] = "invalid";
		status_map[order_status_t::no_deal] = "no_deal";
		status_map[order_status_t::part_deal] = "part_deal";
		status_map[order_status_t::full_deal] = "full_deal";
		status_map[order_status_t::canceling] = "canceling";
		status_map[order_status_t::canceled] = "canceled";
	}

	out << "id: " << order.id() << "\n";
	out << "type: " << type_map[order.type()] << "\n";
	out << "status: " << status_map[order.status()] << "\n";
	out << "price: " << order.price() << "\n";
	out << "amount: " << order.amount() << "\n";
	out << "avg_price: " << order.avg_price() << "\n";
	out << "deal_amount: " << order.deal_amount();

	return out;
}
