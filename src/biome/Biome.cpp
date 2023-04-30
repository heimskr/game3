#include "biome/Biome.h"
#include "biome/Desert.h"
#include "biome/Grassland.h"
#include "biome/Snowy.h"
#include "biome/Volcanic.h"

namespace Game3 {
	std::map<BiomeType, std::shared_ptr<const Biome>> Biome::map {
		{Biome::VOID,      std::make_shared<Biome>(VOID)},
		{Biome::GRASSLAND, std::make_shared<Grassland>()},
		{Biome::VOLCANIC,  std::make_shared<Volcanic>()},
		{Biome::SNOWY,     std::make_shared<Snowy>()},
		{Biome::DESERT,    std::make_shared<Desert>()},
		{Biome::CAVE,      std::make_shared<Biome>(CAVE)},
	};

	void Biome::init(Realm &realm_, int) {
		setRealm(realm_);
	}

	std::map<BiomeType, BiomePtr> Biome::getMap(Realm &realm, int noise_seed) {
		std::map<BiomeType, BiomePtr> out;

		for (const auto &[type, ptr]: map) {
			auto biome = ptr->clone();
			biome->init(realm, noise_seed);
			out.emplace(type, biome);
		}

		return out;
	}
}
