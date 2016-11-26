
#include "order.h"

#include <string>
#include <unordered_map>

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

std::ostream& operator<<(std::ostream& out, const order_t& order)
{
	static std::unordered_map<order_type_t, std::string, EnumClassHash> type_map;
	static std::unordered_map<order_status_t, std::string, EnumClassHash> status_map;

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
	out << "deal_amount: " << order.deal_amount() << "\n";

	return out;
}
