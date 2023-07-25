#include "game/Fluids.h"
#include "net/Buffer.h"

namespace Game3 {
	Fluid::Fluid(Identifier identifier_, Identifier tileset_name, Identifier tilename_, Identifier flask_name):
		NamedRegisterable(std::move(identifier_)),
		tilesetName(std::move(tileset_name)),
		tilename(std::move(tilename_)),
		flaskName(std::move(flask_name)) {}

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

	std::ostream & operator<<(std::ostream &os, const FluidTile &fluid_tile) {
		return os << "FluidTile(" << fluid_tile.id << ", " << fluid_tile.level << ")";
	}

	template <>
	std::string Buffer::getType(const FluidTile &) {
		return getType(uint32_t{});
	}

	template <>
	FluidTile popBuffer<FluidTile>(Buffer &buffer) {
		return FluidTile(popBuffer<uint32_t>(buffer));
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
}
