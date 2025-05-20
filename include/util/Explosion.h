#pragma once

#include "net/Buffer.h"

namespace Game3 {
	class Encodable;
	struct Place;

	struct ExplosionOptions {
		Buffer randomizationParameters{Side::Invalid};
		std::string type = "default";
		Identifier particleType = "base:entity/square_particle";
		std::optional<Identifier> soundEffect = "base:sound/explosion";
		std::optional<float> pitchVariance = 1.2f;
		float radius = 0;
		float damageScale = 1;
		bool destroysTileEntities = false;

		template <typename T>
		ExplosionOptions & operator>>(T &&data) {
			data.decode(randomizationParameters);
			return *this;
		}

		template <typename T>
		ExplosionOptions & operator<<(T &&data) {
			data.encode(randomizationParameters);
			return *this;
		}
	};

	void causeExplosion(const Place &, const ExplosionOptions &);
	void causeExplosion(const Place &, float radius, float damage_scale = 1, bool destroys_tile_entities = false);
}
