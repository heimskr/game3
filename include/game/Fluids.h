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
		Identifier flaskName;

		Fluid() = delete;
		Fluid(Identifier identifier_, Identifier tileset_name, Identifier tilename_, Identifier flask_name = {});
	};

	struct FluidTile {
		static constexpr FluidLevel INFINITE = 65535;
		static constexpr FluidLevel FULL     = 65534;

		FluidID id = 0;
		FluidLevel level = 0;

		FluidTile() = default;
		explicit FluidTile(uint32_t);
		FluidTile(FluidID id_, FluidLevel level_):
			id(id_), level(level_) {}

		explicit operator uint32_t() const;
		explicit operator std::string() const;

		auto operator<=>(const FluidTile &) const = default;
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
