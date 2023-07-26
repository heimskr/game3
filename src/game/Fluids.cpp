#include "game/Fluids.h"
#include "net/Buffer.h"

namespace Game3 {

// Fluid

	Fluid::Fluid(Identifier identifier_, Identifier tileset_name, Identifier tilename_, Identifier flask_name):
		NamedRegisterable(std::move(identifier_)),
		tilesetName(std::move(tileset_name)),
		tilename(std::move(tilename_)),
		flaskName(std::move(flask_name)) {}

// FluidTile

	FluidTile::FluidTile(uint32_t packed):
		id(packed & 0xffff),
		level((packed >> 16) & 0xffff) {}

	FluidTile::operator uint32_t() const {
		return static_cast<uint32_t>(id) | (static_cast<uint32_t>(level) << 16);
	}

	FluidTile::operator std::string() const {
		std::string out = "FluidTile(";
		out += std::to_string(id);
		out += ", ";
		out += std::to_string(level);
		out += ')';
		return out;
	}

	std::ostream & operator<<(std::ostream &os, FluidTile fluid_tile) {
		return os << "FluidTile(" << fluid_tile.id << ", " << fluid_tile.level << ')';
	}

// FluidStack

	FluidStack::FluidStack(FluidID id_, FluidAmount level_):
		id(id_), level(level_) {}

	FluidStack::operator std::string() const {
		std::string out = "FluidStack(";
		out += std::to_string(id);
		out += ", ";
		out += std::to_string(level);
		out += ')';
		return out;
	}

	std::ostream & operator<<(std::ostream &os, FluidStack fluid_stack) {
		return os << "FluidStack(" << fluid_stack.id << ", " << fluid_stack.level << ')';
	}

// Buffer specializations

	template <>
	std::string Buffer::getType(const FluidTile &) {
		return getType(uint32_t{});
	}

	template <>
	std::string Buffer::getType(const FluidStack &) {
		return {'\xe2'};
	}

	template <>
	FluidTile popBuffer<FluidTile>(Buffer &buffer) {
		return FluidTile(popBuffer<uint32_t>(buffer));
	}

	template <>
	FluidStack popBuffer<FluidStack>(Buffer &buffer) {
		FluidStack out;
		buffer >> out;
		return out;
	}

	Buffer & operator+=(Buffer &buffer, const FluidTile &fluid_tile) {
		return buffer += static_cast<uint32_t>(fluid_tile);
	}

	Buffer & operator<<(Buffer &buffer, const FluidTile &fluid_tile) {
		return buffer << static_cast<uint32_t>(fluid_tile);
	}

	Buffer & operator>>(Buffer &buffer, FluidTile &fluid_tile) {
		fluid_tile = FluidTile(buffer.take<uint32_t>());
		return buffer;
	}

	Buffer & operator+=(Buffer &buffer, const FluidStack &fluid_stack) {
		buffer.appendType(fluid_stack);
		buffer += fluid_stack.id;
		buffer += fluid_stack.level;
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const FluidStack &fluid_stack) {
		return buffer += fluid_stack;
	}

	Buffer & operator>>(Buffer &buffer, FluidStack &fluid_stack) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(fluid_stack))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type) + ") in buffer (expected FluidStack)");
		}
		popBuffer(buffer, fluid_stack.id);
		popBuffer(buffer, fluid_stack.level);
		return buffer;
	}

}
