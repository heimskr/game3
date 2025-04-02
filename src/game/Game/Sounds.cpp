#include "data/SoundPath.h"
#include "game/Crop.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "tile/CaveTile.h"
#include "tile/CropTile.h"
#include "tile/DirtTile.h"
#include "tile/FarmlandTile.h"
#include "tile/ForestFloorTile.h"
#include "tile/GrassTile.h"
#include "tile/Tile.h"
#include "tile/TreeTile.h"
#include "util/FS.h"

namespace Game3 {
	const std::filesystem::path * Game::getSound(const Identifier &identifier) {
		if (auto sound_path = registry<SoundRegistry>().maybe(identifier)) {
			return &sound_path->path;
		}
		return nullptr;
	}

	size_t Game::addSounds(const std::filesystem::path &dir) {
		auto &reg = registry<SoundRegistry>();
		size_t added = 0;

		for (const auto &entry: std::filesystem::directory_iterator(dir)) {
			if (!entry.is_directory()) {
				continue;
			}

			boost::json::value json = boost::json::parse(readFile(entry.path() / "sound.json"));
			Identifier id(json.at("id").as_string());
			SoundPath sound_path(id, entry.path() / "sound.opus");
			reg.add(std::move(id), std::move(sound_path));
			++added;
		}

		return added;
	}
}
