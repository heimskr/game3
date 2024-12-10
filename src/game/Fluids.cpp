#include "game/Fluids.h"
#include "game/Game.h"
#include "net/Buffer.h"

#include <boost/json/fwd.hpp>

namespace Game3 {

// Fluid

	Fluid::Fluid(Identifier identifier_, std::string name_, Identifier tileset_name, Identifier tilename_, Color color, Identifier flask_name):
		NamedRegisterable(std::move(identifier_)),
		name(std::move(name_)),
		tilesetName(std::move(tileset_name)),
		tilename(std::move(tilename_)),
		flaskName(std::move(flask_name)),
		color(color) {}

// FluidTile

	static_assert(sizeof(FluidTile::id)    == 2);
	static_assert(sizeof(FluidTile::level) == 2);

	FluidTile::FluidTile(FluidInt packed):
		id(packed & 0xffff),
		level((packed >> 16) & 0xffff),
		infinite((packed >> 32) & 1) {}

	FluidTile::operator FluidInt() const {
		return static_cast<FluidInt>(id) | (static_cast<FluidInt>(level) << 16) | (static_cast<FluidInt>(infinite) << 32);
	}

	FluidTile::operator std::string() const {
		std::string out = "FluidTile(";
		out += std::to_string(id);
		out += ", ";
		out += std::to_string(level);
		out += ", ";
		out += infinite? 'T' : 'F';
		out += ')';
		return out;
	}

// FluidStack

	FluidStack::FluidStack(FluidID id_, FluidAmount amount_):
		id(id_), amount(amount_) {}

	FluidStack::operator std::string() const {
		std::string out = "FluidStack(";
		out += std::to_string(id);
		out += ", ";
		out += std::to_string(amount);
		out += ')';
		return out;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const FluidStack &stack, const GamePtr &game) {
		const auto &registry = game->registry<FluidRegistry>();
		auto &array = json.emplace_array();
		array.emplace_back(boost::json::value_from(registry.at(static_cast<size_t>(stack.id))->identifier));
		array.emplace_back(stack.amount);
	}

	FluidStack tag_invoke(boost::json::value_to_tag<FluidStack>, const boost::json::value &json, const GamePtr &game) {
		const auto &registry = game->registry<FluidRegistry>();
		auto id = static_cast<FluidID>(registry.at(boost::json::value_to<Identifier>(json.at(0)))->registryID);
		return {id, boost::json::value_to<FluidAmount>(json.at(1))};
	}

// Buffer specializations

	template <>
	std::string Buffer::getType(const FluidTile &, bool) {
		return getType(FluidInt{}, false);
	}

	template <>
	std::string Buffer::getType(const FluidStack &, bool) {
		return {'\xe2'};
	}

	template <>
	FluidTile popBuffer<FluidTile>(Buffer &buffer) {
		return FluidTile(popBuffer<FluidInt>(buffer));
	}

	template <>
	FluidStack popBuffer<FluidStack>(Buffer &buffer) {
		FluidStack out;
		buffer >> out;
		return out;
	}

	Buffer & operator+=(Buffer &buffer, const FluidTile &fluid_tile) {
		return buffer += static_cast<FluidInt>(fluid_tile);
	}

	Buffer & operator<<(Buffer &buffer, const FluidTile &fluid_tile) {
		return buffer << static_cast<FluidInt>(fluid_tile);
	}

	Buffer & operator>>(Buffer &buffer, FluidTile &fluid_tile) {
		fluid_tile = FluidTile(buffer.take<FluidInt>());
		return buffer;
	}

	Buffer & operator+=(Buffer &buffer, const FluidStack &fluid_stack) {
		buffer.appendType(fluid_stack, false);
		buffer << fluid_stack.id;
		buffer << fluid_stack.amount;
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const FluidStack &fluid_stack) {
		return buffer += fluid_stack;
	}

	Buffer & operator>>(Buffer &buffer, FluidStack &fluid_stack) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(fluid_stack, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected FluidStack)");
		}
		fluid_stack.id = buffer.take<FluidID>();
		fluid_stack.amount = buffer.take<FluidAmount>();
		return buffer;
	}

}
