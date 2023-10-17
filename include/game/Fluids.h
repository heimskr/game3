#pragma once

#include "Chunk.h"
#include "types/Types.h"
#include "data/Identifier.h"

#include <nlohmann/json_fwd.hpp>
#include <ostream>
#include <string>

namespace Game3 {
	class Buffer;
	class Game;

	struct Fluid: NamedRegisterable {
		std::string name;
		Identifier tilesetName;
		Identifier tilename;
		Identifier flaskName;

		Fluid() = delete;
		Fluid(Identifier identifier_, std::string name_, Identifier tileset_name, Identifier tilename_, Identifier flask_name = {});
	};

	using FluidInt = uint64_t;

	struct FluidTile {
		static constexpr FluidLevel FULL = 1000;

		FluidID id = 0;
		FluidLevel level = 0;
		bool infinite = false;

		FluidTile() = default;
		explicit FluidTile(FluidInt);
		FluidTile(FluidID id_, FluidLevel level_, bool infinite_ = false):
			id(id_), level(level_), infinite(infinite_) {}

		explicit operator FluidInt() const;
		explicit operator std::string() const;

		auto operator<=>(const FluidTile &) const = default;

		inline bool isInfinite() const { return infinite; }
	};

	std::ostream & operator<<(std::ostream &, FluidTile);

	struct FluidStack {
		FluidID id = 0;
		FluidAmount amount = 0;

		FluidStack() = default;
		FluidStack(FluidID, FluidAmount);

		explicit operator std::string() const;

		auto operator<=>(const FluidStack &) const = default;

		static FluidStack fromJSON(const Game &, const nlohmann::json &);
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

	using FluidChunk = Chunk<FluidTile>;
}
