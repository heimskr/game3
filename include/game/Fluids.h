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
		static constexpr FluidLevel FULL     = 1000;
		static constexpr FluidLevel INFINITE = FULL + 1;

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

	std::ostream & operator<<(std::ostream &, FluidTile);

	struct FluidStack {
		FluidID id = 0;
		FullFluidLevel level = 0;

		FluidStack() = default;
		FluidStack(FluidID, FullFluidLevel);

		explicit operator std::string() const;

		auto operator<=>(const FluidStack &) const = default;
	};

	std::ostream & operator<<(std::ostream &, FluidStack);

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	FluidTile popBuffer<FluidTile>(Buffer &);
	template <>
	FluidStack popBuffer<FluidStack>(Buffer &);

	Buffer & operator+=(Buffer &, const FluidTile &);
	Buffer & operator<<(Buffer &, const FluidTile &);
	Buffer & operator>>(Buffer &, FluidTile &);

	Buffer & operator+=(Buffer &, const FluidStack &);
	Buffer & operator<<(Buffer &, const FluidStack &);
	Buffer & operator>>(Buffer &, FluidStack &);
}
