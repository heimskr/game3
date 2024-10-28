#include "entity/Crab.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	namespace {
		constexpr std::size_t MAX_SELECTION_ATTEMPTS = 16;
	}

	std::map<LongGene::ValueType, Identifier> Crab::breeds{
		{0, "base:texture/crab"},
		{8, "base:texture/crab_blue"},
	};

	bool Crab::canAbsorbGenes(const nlohmann::json &genes) const {
		return checkGenes(genes, {"species", "breed"});
	}

	void Crab::absorbGenes(const nlohmann::json &genes) {
		absorbGene(breed, genes, "breed");
		texture = nullptr;
	}

	void Crab::iterateGenes(const std::function<void(Gene &)> &function) {
		function(species);
		function(breed);
	}

	void Crab::iterateGenes(const std::function<void(const Gene &)> &function) const {
		function(species);
		function(breed);
	}

	void Crab::render(const RendererContext &renderers) {
		if (!texture) {
			// It's assumed they're all the same variety.
			Identifier texture_id;

			if (auto iter = breeds.find(breed.getValue()); iter != breeds.end()) {
				texture_id = iter->second;
			} else {
				texture_id = breeds.at(0);
			}

			texture = getGame()->registry<TextureRegistry>().at(texture_id);
			assert(texture);
		}

		Animal::render(renderers);
	}

	bool Crab::wander() {
		if (!attemptingWander.exchange(true)) {
			increaseUpdateCounter();
			const auto [row, column] = position.copyBase();
			return threadPool.add([weak = weak_from_this(), row = row, column = column](ThreadPool &, size_t) {
				if (auto crab = std::dynamic_pointer_cast<Crab>(weak.lock())) {
					RealmPtr realm = crab->getRealm();
					Position start_position = crab->getPosition();
					Position goal = start_position;

					bool in_water = false;
					if (std::optional<FluidTile> fluid = realm->tryFluid(start_position); fluid->level > 0) {
						in_water = true;
					}

					const TileID sand = realm->getTileset()["base:tile/sand"_id];

					for (std::size_t attempt = 0; attempt < MAX_SELECTION_ATTEMPTS; ++attempt) {
						goal = {
							threadContext.random(int64_t(row    - crab->wanderRadius), int64_t(row    + crab->wanderRadius)),
							threadContext.random(int64_t(column - crab->wanderRadius), int64_t(column + crab->wanderRadius))
						};

						// The goal position has to have sand on the terrain layer, and crabs can move either:
						// - from sand to sand,
						// - from sand to water,
						// or
						// - from water to sand.
						if (realm->tryTile(Layer::Terrain, goal) == sand && (!in_water || !realm->hasFluid(goal))) {
							crab->pathfind(goal, PATHFIND_MAX);
							break;
						}
					}

					crab->attemptingWander = false;
				}
			});
		}

		return false;
	}

	void Crab::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << breed;
	}

	void Crab::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> breed;
		texture = nullptr;
	}

	void Crab::setBreed(LongGene::ValueType new_breed) {
		breed.setValue(new_breed);
		texture = nullptr;
	}

	LongGene::ValueType Crab::sampleBreed() {
		return threadContext.random(0, 10);
	}
}
