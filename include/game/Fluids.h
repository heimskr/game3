#pragma once

#include "Types.h"
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

		inline bool isInfinite() const { return INFINITE <= level; }
	};

	std::ostream & operator<<(std::ostream &, FluidTile);

	class FluidStack {
		public:
			FluidID id = 0;
			FluidAmount amount = 0;

			FluidStack() = default;
			FluidStack(const std::shared_ptr<Game> &, FluidID, FluidAmount);

			explicit operator std::string() const;

			auto operator<=>(const FluidStack &) const = default;

			static FluidStack fromJSON(const Game &, const nlohmann::json &);

			inline const Game & getGame() { auto locked = game.lock(); assert(game); return *game; }

		private:
			std::weak_ptr<Game> game;
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
