#include "biome/Biome.h"
#include "biome/Desert.h"
#include "biome/Grassland.h"
#include "biome/Snowy.h"
#include "biome/Volcanic.h"

namespace Game3 {
	const std::map<BiomeType, std::shared_ptr<const Biome>> & Biome::getMap() {
		static const std::map<BiomeType, std::shared_ptr<const Biome>> map {
			{VOID_REALM, std::make_shared<Biome>(VOID_REALM)},
			{GRASSLAND,  std::make_shared<Grassland>()},
			{VOLCANIC,   std::make_shared<Volcanic>()},
			{SNOWY,      std::make_shared<Snowy>()},
			{DESERT,     std::make_shared<Desert>()},
			{CAVE,       std::make_shared<Biome>(CAVE)},
		};

		return map;
	};

	void Biome::init(const std::shared_ptr<Realm> &realm, int) {
		setRealm(realm);
	}

	double Biome::generate(Index, Index, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &, double) {
		return 0.0;
	}

	std::shared_ptr<Realm> Biome::getRealm() const {
		std::shared_ptr<Realm> locked = weakRealm.lock();
		assert(locked != nullptr);
		return locked;
	}

	void Biome::setRealm(const std::shared_ptr<Realm> &realm) {
		weakRealm = realm;
	}

	std::map<BiomeType, BiomePtr> Biome::getMap(const std::shared_ptr<Realm> &realm, int noise_seed) {
		std::map<BiomeType, BiomePtr> out;

		for (const auto &[type, ptr]: getMap()) {
			auto biome = ptr->clone();
			biome->init(realm, noise_seed);
			out.emplace(type, biome);
		}

		return out;
	}
}
