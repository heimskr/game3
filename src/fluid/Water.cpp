#include "entity/LivingEntity.h"
#include "fluid/FreshWater.h"
#include "fluid/Seawater.h"
#include "fluid/Water.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "statuseffect/Burning.h"

#include <deque>

namespace Game3 {
	Water::Water(Fluid &&fluid):
		Fluid(std::move(fluid)) {}

	void Water::onCollision(const std::shared_ptr<LivingEntity> &target) {
		if (target->getSide() == Side::Server) {
			target->removeStatusEffect(Burning::ID());
		}
	}

	FluidPtr Water::resolve(const Place &place, size_t count) {
		constexpr size_t threshold = 256; // Reaching this will select seawater.

		if (count == 0) {
			count = threshold + 1;
		}

		std::deque<Position> queue;
		std::unordered_set<Position> visited;
		visited.reserve(count);

		TileProvider &provider = place.realm->tileProvider;
		std::unique_lock lock{provider.fluidMutex};

		auto find_chunk = [&](ChunkPosition chunk_position) -> FluidChunk * {
			if (auto iter = provider.fluidMap.find(chunk_position); iter != provider.fluidMap.end()) {
				return &iter->second;
			}

			return nullptr;
		};

		ChunkPosition chunk_position{place.position};
		FluidChunk *chunk = find_chunk(chunk_position);

		if (chunk == nullptr) {
			return nullptr;
		}

		GamePtr game = place.getGame();

		auto get_fluid = [&](Position position) -> FluidTile * {
			if (ChunkPosition new_chunk_position{position}; new_chunk_position != chunk_position) {
				chunk_position = new_chunk_position;
				chunk = find_chunk(chunk_position);
			}

			if (chunk == nullptr) {
				return nullptr;
			}

			return &TileProvider::access(*chunk, TileProvider::remainder(position.row), TileProvider::remainder(position.column));
		};

		enum class VisitResult: uint8_t {
			AlreadyVisited,
			NoWater,
			FoundWater,
			FoundFreshWater,
			FoundSeawater,
		};

		using enum VisitResult;

		const auto water = game->getFluid(Water::ID());
		const auto fresh_water = game->getFluid(FreshWater::ID());
		const auto seawater = game->getFluid(Seawater::ID());

		auto initial_visit = [&](Position position) {
			if (visited.contains(position)) {
				return AlreadyVisited;
			}

			const FluidTile *tile = get_fluid(position);

			if (!tile) {
				return NoWater;
			}

			FluidPtr fluid = game->getFluid(tile->id);

			if (!fluid) {
				return NoWater;
			}

			if (fluid == fresh_water) {
				return FoundFreshWater;
			}

			if (fluid == seawater) {
				return FoundSeawater;
			}

			if (fluid != water) {
				return NoWater;
			}

			visited.emplace(position);

			for (Direction direction: ALL_DIRECTIONS) {
				Position offset = position + direction;
				if (!visited.contains(offset)) {
					queue.emplace_back(offset);
				}
			}

			return FoundWater;
		};

		queue.emplace_back(place.position);

		FluidPtr decision = nullptr;

		size_t water_found = 0;

		for (; water_found < count && !queue.empty();) {
			Position position = queue.front();
			queue.pop_front();

			switch (initial_visit(position)) {
				case FoundFreshWater:
					decision = fresh_water;
					goto initial_done;

				case FoundSeawater:
					decision = seawater;
					goto initial_done;

				case FoundWater:
					++water_found;
					break;

				case AlreadyVisited:
				case NoWater:
					// Do nothing; look for more water.
					break;
			}
		} initial_done:

		if (!decision) {
			decision = water_found >= threshold? seawater : fresh_water;
		}

		for (Position position: visited) {
			if (FluidTile *tile = get_fluid(position)) {
				tile->id = decision->registryID;
			}
		}

		return decision;
	}
}
