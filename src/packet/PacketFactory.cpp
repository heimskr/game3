#include "packet/PacketFactory.h"

namespace Game3 {
	PacketFactory::PacketFactory(PacketID number_, decltype(function) function_):
		NumericRegisterable(number_), function(std::move(function_)) {}

	std::shared_ptr<Packet> PacketFactory::operator()() {
		if (!function)
			throw std::logic_error("PacketFactory is missing a function");

		return function();
	}
}
