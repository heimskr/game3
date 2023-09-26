#pragma once

#include "Directions.h"
#include "Types.h"
#include "container/DirectionalContainer.h"
#include "tileentity/TileEntity.h"

#include <optional>

namespace Game3 {
	class PipeNetwork;

	constexpr std::array<PipeType, 3> PIPE_TYPES{PipeType::Item, PipeType::Fluid, PipeType::Energy};

	template <typename T>
	class PipeTuple {
		public:
			T item{};
			T fluid{};
			T energy{};

			PipeTuple() = default;
			PipeTuple(T item_, T fluid_, T energy_):
				item(std::move(item_)), fluid(std::move(fluid_)), energy(std::move(energy_)) {}

			T & operator[](PipeType type) {
				switch (type) {
					case PipeType::Item:   return item;
					case PipeType::Fluid:  return fluid;
					case PipeType::Energy: return energy;
					default: throw std::invalid_argument("Invalid PipeType");
				}
			}

			const T & operator[](PipeType type) const {
				switch (type) {
					case PipeType::Item:   return item;
					case PipeType::Fluid:  return fluid;
					case PipeType::Energy: return energy;
					default: throw std::invalid_argument("Invalid PipeType");
				}
			}
	};

	class Pipe: public TileEntity {
		friend class PipeLoader;

		private:
			static Identifier ItemCorner()   { return {"base", "tile/item_pipe"};   }
			static Identifier FluidCorner()  { return {"base", "tile/fluid_pipe"};  }
			static Identifier EnergyCorner() { return {"base", "tile/energy_pipe"}; }
			static Identifier Corner(PipeType);

			static Identifier ItemExtractorsCorner()   { return {"base", "tile/item_extractors_"};   }
			static Identifier FluidExtractorsCorner()  { return {"base", "tile/fluid_extractors_"};  }
			static Identifier EnergyExtractorsCorner() { return {"base", "tile/energy_extractors"}; }
			static Identifier ExtractorsCorner(PipeType);

			PipeTuple<Directions> directions;
			PipeTuple<Directions> extractors;
			PipeTuple<std::optional<TileID>> tileIDs;
			PipeTuple<bool> present;
			PipeTuple<std::shared_ptr<PipeNetwork>> networks;
			PipeTuple<std::optional<TileID>> extractorsCorners;
			PipeTuple<bool> loaded;

			void updateTileID(PipeType);
			bool get(PipeType, Direction);
			void set(PipeType, Direction, bool);
			void setExtractor(PipeType, Direction, bool);

		public:
			static Identifier ID() { return {"base", "te/pipe"}; }

			Pipe() = default;
			Pipe(Position);

			DirectionalContainer<std::shared_ptr<Pipe>> getConnected(PipeType) const;
			std::shared_ptr<Pipe> getConnected(PipeType, Direction) const;

			void tick(Game &, float) override;
			void render(SpriteRenderer &) override;
			void onNeighborUpdated(Position offset) override;

			inline auto & getDirections() { return directions; }
			inline const auto & getDirections() const { return directions; }

			inline auto & getExtractors() { return extractors; }
			inline const auto & getExtractors() const { return extractors; }

			void toggle(PipeType, Direction);
			void toggleExtractor(PipeType, Direction);

			void setPresent(PipeType, bool);
			inline bool getPresent(PipeType pipe_type) const { return present[pipe_type]; }

			/** Returns whether there exists some path between this pipe and the given pipe. */
			bool reachable(PipeType, const std::shared_ptr<Pipe> &);

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			/** Implicitly marks the pipe as loaded. */
			void setNetwork(PipeType, const std::shared_ptr<PipeNetwork> &);
			std::shared_ptr<PipeNetwork> getNetwork(PipeType) const;

			void onSpawn() override;
			void onRemove() override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
	};

	template <typename T>
	Buffer & operator+=(Buffer &buffer, const PipeTuple<T> &tuple) {
		return ((buffer += tuple.item) += tuple.fluid) += tuple.energy;
	}

	template <typename T>
	Buffer & operator<<(Buffer &buffer, const PipeTuple<T> &tuple) {
		return buffer << tuple.item << tuple.fluid << tuple.energy;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, PipeTuple<T> &tuple) {
		return buffer >> tuple.item >> tuple.fluid >> tuple.energy;
	}
}
