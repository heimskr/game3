#include "biome/Biome.h"
#include "biome/Grassland.h"
#include "biome/Snowy.h"
#include "biome/Volcanic.h"

namespace Game3 {
	std::map<BiomeType, std::shared_ptr<const Biome>> Biome::map {
		{Biome::VOID,      std::make_shared<Biome>(VOID)},
		{Biome::GRASSLAND, std::make_shared<Grassland>()},
		{Biome::VOLCANIC,  std::make_shared<Volcanic>()},
		{Biome::SNOWY,     std::make_shared<Snowy>()},
		{Biome::DESERT,    std::make_shared<Biome>(DESERT)},
		{Biome::CAVE,      std::make_shared<Biome>(CAVE)},
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
