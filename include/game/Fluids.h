#pragma once

#include <ostream>
#include <string>

#include "Types.h"
#include "data/Identifier.h"

namespace Game3 {
	class Buffer;

	struct Fluid: NamedRegisterable {
		Identifier tilesetName;
		Identifier tilename;

		Fluid() = delete;
		Fluid(Identifier identifier_, Identifier tileset_name, Identifier tilename_);
	};

	struct FluidTile {
		FluidID id = 0;
		FluidLevel level = 0;

		FluidTile() = default;
		explicit FluidTile(uint32_t);
		FluidTile(FluidID id_, FluidLevel level_):
			id(id_), level(level_) {}

		explicit operator uint32_t() const;
		explicit operator std::string() const;
	};

	std::ostream & operator<<(std::ostream &, const FluidTile &);

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	FluidTile popBuffer<FluidTile>(Buffer &);

	Buffer & operator+=(Buffer &, const FluidTile &);
	Buffer & operator<<(Buffer &, const FluidTile &);
	Buffer & operator>>(Buffer &, FluidTile &);
}
