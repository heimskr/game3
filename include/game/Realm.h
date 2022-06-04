#pragma once

#include <memory>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "Tilemap.h"
#include "Types.h"
#include "game/TileEntity.h"
#include "ui/ElementBufferedRenderer.h"

namespace Game3 {
	class Realm {
		public:
			int id;
			std::shared_ptr<Tilemap> tilemap1, tilemap2, tilemap3;
			ElementBufferedRenderer renderer1, renderer2, renderer3;
			std::unordered_map<Index, std::shared_ptr<TileEntity>> tileEntities;

			Realm() = default;
			Realm(int id_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_);
			Realm(int id_, const std::shared_ptr<Tilemap> &tilemap1_);

			void render(int width, int height, const nanogui::Vector2f &center, float scale);
			void reupload();
			void rebind();
			void generate(int seed = 666, double noise_zoom = 100., double noise_threshold = -0.15);
			void createTown(size_t index, size_t width, size_t height, size_t pad);
	};

	void to_json(nlohmann::json &, const Realm &);
	void from_json(const nlohmann::json &, Realm &);
}
