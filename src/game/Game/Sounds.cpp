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
		if (auto sound_path = registry<SoundRegistry>().maybe(identifier))
			return &sound_path->path;
		return nullptr;
	}

	size_t Game::addSounds(const std::filesystem::path &dir) {
		auto &reg = registry<SoundRegistry>();
		size_t added = 0;

		for (const auto &entry: std::filesystem::directory_iterator(dir)) {
			if (!entry.is_directory())
				continue;

			nlohmann::json json = nlohmann::json::parse(readFile(entry.path() / "sound.json"));
			const Identifier id = json.at("id");
			reg.add(id, SoundPath(id, entry.path() / "sound.opus"));
			++added;
		}

		return added;
	}
}
