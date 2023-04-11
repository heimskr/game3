#include "realm/RealmFactory.h"

namespace Game3 {
	RealmFactory::RealmFactory(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)), function(std::move(function_)) {}

	std::shared_ptr<Realm> RealmFactory::operator()(Game &game) {
		if (!function)
			throw std::logic_error("RealmFactory is missing a function");

		return function(game);
	}
}
