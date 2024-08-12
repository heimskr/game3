#include "biome/Biome.h"
#include "biome/Desert.h"
#include "biome/Grassland.h"
#include "biome/Snowy.h"
#include "biome/Volcanic.h"

namespace Game3 {
	const std::map<BiomeType, std::shared_ptr<const Biome>> & Biome::getMap() {
		static const std::map<BiomeType, std::shared_ptr<const Biome>> map {
			{Biome::VOID,      std::make_shared<Biome>(VOID)},
			{Biome::GRASSLAND, std::make_shared<Grassland>()},
			{Biome::VOLCANIC,  std::make_shared<Volcanic>()},
			{Biome::SNOWY,     std::make_shared<Snowy>()},
			{Biome::DESERT,    std::make_shared<Desert>()},
			{Biome::CAVE,      std::make_shared<Biome>(CAVE)},
		};

		return map;
	};

	void Biome::init(Realm &realm_, int) {
		setRealm(realm_);
	}

	std::map<BiomeType, BiomePtr> Biome::getMap(Realm &realm, int noise_seed) {
		std::map<BiomeType, BiomePtr> out;

		for (const auto &[type, ptr]: getMap()) {
			auto biome = ptr->clone();
			biome->init(realm, noise_seed);
			out.emplace(type, biome);
		}

		return out;
	}
}
