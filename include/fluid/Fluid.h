#pragma once

#include "data/Identifier.h"
#include "game/Chunk.h"
#include "graphics/Color.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <ostream>
#include <string>

namespace Game3 {
	class BasicBuffer;
	class Buffer;
	class Game;
	class LivingEntity;

	class Fluid: public NamedRegisterable {
		public:
			std::string name;
			Identifier tilesetName;
			Identifier tilename;
			Identifier flaskName;
			Color color;

			Fluid(Identifier identifier, std::string name, Identifier tilesetName, Identifier tilename, Color color, Identifier flaskName = {});

			virtual ~Fluid();

			virtual void onCollision(const std::shared_ptr<LivingEntity> &);

			/** For certain fluids, converts their indeterminate variant to a determinate variant at a given location using a flood fill algorithm. */
			virtual std::shared_ptr<Fluid> resolve(const Place &, size_t count = 0);
	};

	using FluidInt = uint64_t;
	using FluidPtr = std::shared_ptr<Fluid>;

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

	struct FluidStack {
		FluidID id = 0;
		FluidAmount amount = 0;

		FluidStack() = default;
		FluidStack(FluidID, FluidAmount);

		explicit operator std::string() const;

		auto operator<=>(const FluidStack &) const = default;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const FluidStack &, const GamePtr &);
	FluidStack tag_invoke(boost::json::value_to_tag<FluidStack>, const boost::json::value &, const GamePtr &);

	template <typename T>
	T popBuffer(BasicBuffer &);
	template <>
	FluidTile popBuffer<FluidTile>(BasicBuffer &);
	template <>
	FluidStack popBuffer<FluidStack>(BasicBuffer &);

	Buffer & operator+=(Buffer &, const FluidTile &);
	Buffer & operator<<(Buffer &, const FluidTile &);
	BasicBuffer & operator>>(BasicBuffer &, FluidTile &);

	Buffer & operator+=(Buffer &, const FluidStack &);
	Buffer & operator<<(Buffer &, const FluidStack &);
	BasicBuffer & operator>>(BasicBuffer &, FluidStack &);

	using FluidChunk = Chunk<FluidTile>;
}

template <>
struct std::formatter<Game3::FluidTile> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::FluidTile &fluid_tile, auto &ctx) const {
		return std::format_to(ctx.out(), "FluidTile({}, {})", fluid_tile.id, fluid_tile.level);
	}
};

template <>
struct std::formatter<Game3::FluidStack> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::FluidStack &fluid_stack, auto &ctx) const {
		return std::format_to(ctx.out(), "FluidStack({}, {})", fluid_stack.id, fluid_stack.amount);
	}
};
