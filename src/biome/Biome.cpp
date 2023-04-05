#include "biome/Biome.h"
#include "biome/Grassland.h"

namespace Game3 {
	std::map<BiomeType, std::shared_ptr<const Biome>> Biome::map {
		{Biome::VOID_ID,      std::make_shared<Biome>(VOID_ID)},
		{Biome::GRASSLAND_ID, std::make_shared<Grassland>()},
		{Biome::VOLCANIC_ID,  std::make_shared<Biome>(VOLCANIC_ID)},
		{Biome::SNOWY_ID,     std::make_shared<Biome>(SNOWY_ID)},
		{Biome::DESERT_ID,    std::make_shared<Biome>(DESERT_ID)},
		{Biome::CAVE_ID,      std::make_shared<Biome>(CAVE_ID)},
	};

	void Biome::init(Realm &realm_, int, const std::shared_ptr<double[]> &saved_noise) {
		setRealm(realm_);
		savedNoise = saved_noise;
	}

	std::map<BiomeType, BiomePtr> Biome::getMap(Realm &realm, int noise_seed, const std::shared_ptr<double[]> &saved_noise) {
		std::map<BiomeType, BiomePtr> out;

		for (const auto &[type, ptr]: map) {
			auto biome = ptr->clone();
			biome->init(realm, noise_seed, saved_noise);
			out.emplace(type, biome);
		}

		return out;
	}
}
