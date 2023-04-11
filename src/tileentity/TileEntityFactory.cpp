#include "tileentity/TileEntityFactory.h"

namespace Game3 {
	TileEntityFactory::TileEntityFactory(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)), function(std::move(function_)) {}

	std::shared_ptr<TileEntity> TileEntityFactory::operator()(Game &game) {
		if (!function)
			throw std::logic_error("TileEntityFactory is missing a function");

		return function(game);
	}
}
