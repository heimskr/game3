#pragma once

#include "types/Directions.h"
#include "types/Types.h"
#include "container/DirectionalContainer.h"
#include "pipes/ItemFilter.h"
#include "tileentity/TileEntity.h"

#include <optional>
#include <utility>

namespace Game3 {
	class PipeNetwork;

	constexpr std::array<Substance, 4> PIPE_TYPES{Substance::Item, Substance::Fluid, Substance::Energy, Substance::Data};

	template <typename T>
	class PipeTuple {
		public:
			T item{};
			T fluid{};
			T energy{};
			T data{};

			PipeTuple() = default;
			PipeTuple(T item_, T fluid_, T energy_, T data_):
				item(std::move(item_)), fluid(std::move(fluid_)), energy(std::move(energy_)), data(std::move(data_)) {}

			T & operator[](Substance type) {
				switch (type) {
					case Substance::Item:   return item;
					case Substance::Fluid:  return fluid;
					case Substance::Energy: return energy;
					case Substance::Data:   return data;
					default: throw std::invalid_argument("Invalid Substance");
				}
			}

			const T & operator[](Substance type) const {
				switch (type) {
					case Substance::Item:   return item;
					case Substance::Fluid:  return fluid;
					case Substance::Energy: return energy;
					case Substance::Data:   return data;
					default: throw std::invalid_argument("Invalid Substance");
				}
			}
	};

	class Pipe: public TileEntity {
		friend class PipeLoader;
		friend class PipeNetwork;

		private:
			static Identifier ItemCorner()   { return {"base", "tile/item_pipe"};   }
			static Identifier FluidCorner()  { return {"base", "tile/fluid_pipe"};  }
			static Identifier EnergyCorner() { return {"base", "tile/energy_pipe"}; }
			static Identifier DataCorner()   { return {"base", "tile/data_pipe"};   }
			static Identifier Corner(Substance);

			static Identifier ItemExtractorsCorner()   { return {"base", "tile/item_extractors"};   }
			static Identifier FluidExtractorsCorner()  { return {"base", "tile/fluid_extractors"};  }
			static Identifier EnergyExtractorsCorner() { return {"base", "tile/energy_extractors"}; }
			static Identifier DataExtractorsCorner()   { return {"base", "tile/data_extractors"};   }
			static Identifier ExtractorsCorner(Substance);

			PipeTuple<Directions> directions;
			PipeTuple<Directions> extractors;
			PipeTuple<std::optional<TileID>> tileIDs;
			PipeTuple<bool> present;
			PipeTuple<std::shared_ptr<PipeNetwork>> networks;
			PipeTuple<std::optional<TileID>> extractorsCorners;
			PipeTuple<bool> loaded;
			PipeTuple<bool> dying;

			void updateTileID(Substance);
			bool get(Substance, Direction);
			void set(Substance, Direction, bool);
			void setExtractor(Substance, Direction, bool);

		public:
			static Identifier ID() { return {"base", "te/pipe"}; }

			DirectionalContainer<ItemFilterPtr> itemFilters;

			Pipe();
			Pipe(Position);

			DirectionalContainer<std::shared_ptr<Pipe>> getConnected(Substance) const;
			std::shared_ptr<Pipe> getConnected(Substance, Direction) const;

			void tick(const TickArgs &) override;
			void render(SpriteRenderer &) override;
			void onNeighborUpdated(Position offset) override;

			inline auto & getDirections() { return directions; }
			inline const auto & getDirections() const { return directions; }

			inline auto & getExtractors() { return extractors; }
			inline const auto & getExtractors() const { return extractors; }

			void toggle(Substance, Direction);
			void toggleExtractor(Substance, Direction);

			void setPresent(Substance, bool);
			inline bool getPresent(Substance pipe_type) const { return present[pipe_type]; }

			std::pair<std::shared_ptr<Pipe>, std::shared_ptr<PipeNetwork>> getNeighbor(Substance, Direction) const;

			/** Returns whether there exists some path between this pipe and the given pipe. */
			bool reachable(Substance, const std::shared_ptr<Pipe> &);

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			/** Implicitly marks the pipe as loaded. */
			void setNetwork(Substance, const std::shared_ptr<PipeNetwork> &);
			std::shared_ptr<PipeNetwork> getNetwork(Substance) const;

			void onSpawn() override;
			void onRemove() override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;

			/** Attaches the pipe to adjacent machines and pipes. */
			void autopipe(Substance);
	};

	template <typename T>
	Buffer & operator+=(Buffer &buffer, const PipeTuple<T> &tuple) {
		return (((buffer += tuple.item) += tuple.fluid) += tuple.energy) += tuple.data;
	}

	template <typename T>
	Buffer & operator<<(Buffer &buffer, const PipeTuple<T> &tuple) {
		return buffer << tuple.item << tuple.fluid << tuple.energy << tuple.data;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, PipeTuple<T> &tuple) {
		return buffer >> tuple.item >> tuple.fluid >> tuple.energy >> tuple.data;
	}
}
