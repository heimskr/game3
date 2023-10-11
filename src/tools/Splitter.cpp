#include "config.h"
#include "App.h"
#include "Log.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "item/Item.h"
#include "tools/Flasker.h"
#include "util/FS.h"
#include "util/Util.h"

#include <fstream>
#include <nlohmann/json.hpp>

#ifdef USING_VCPKG
#include <stb_image.h>
#include <stb_image_write.h>
#else
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#endif

namespace Game3 {
	void splitter() {
		auto game = Game::create(Side::Client, nullptr);

		const std::unordered_map<Identifier, std::string> credits{
			{"base:texture/palisade",    "https://palisadestudio.itch.io/"},
			{"base:texture/shortsword",  "https://merchant-shade.itch.io/16x16-mini-world-sprites"},
			{"base:texture/potions",     "https://merchant-shade.itch.io/16x16-mixed-rpg-icons"},
			{"base:texture/custom",      "Fayabella"},
			{"base:texture/tinywonder",  "https://butterymilk.itch.io/tiny-wonder-farm-asset-pack"},
			{"base:texture/items",       "https://kyrise.itch.io/kyrises-free-16x16-rpg-icon-pack"},
			{"base:texture/shade",       "https://merchant-shade.itch.io/16x16-mini-world-sprites"},
			{"base:texture/tileset",     "https://merchant-shade.itch.io/16x16-mini-world-sprites"},
			{"base:texture/consumables", "https://merchant-shade.itch.io/16x16-mixed-rpg-icons"},
			{"base:texture/mushrooms",   "https://pixerelia.itch.io/vf-edible-mushrooms"},
			{"base:texture/kazzter",     "https://kazzter-k.itch.io/kazzter-16-rpg-icon-pack"},
			{"base:texture/indfor",      "https://github.com/InnovativeOnlineIndustries/Industrial-Foregoing/"},
			{"base:texture/interiors",   "https://limezu.itch.io/moderninteriors"},
		};

		// The original tileset omnipng contains some Fayabella sprites.
		const std::unordered_set<std::string> credit_override_fayabella{
			"ace_flag", "centrifuge", "chemical_reactor", "energy_pipe",
			"flower2_black", "flower2_blue", "flower2_green", "flower2_orange", "flower2_purple", "flower2_red", "flower2_white", "flower2_yellow",
			"flower4_black", "flower4_blue", "flower4_green", "flower4_orange", "flower4_purple", "flower4_red", "flower4_white", "flower4_yellow",
			"fluid_pipe", "geothermal_generator", "item_pipe", "millstone", "nb_flag", "pride_flag", "pump", "purifier", "tank",
		};

		// And some from Vadim.
		const std::unordered_set<std::string> credit_override_vadim{
			"flower1_black", "flower1_blue", "flower1_green", "flower1_orange", "flower1_purple", "flower1_red", "flower1_white", "flower1_yellow",
			"flower3_black", "flower3_blue", "flower3_green", "flower3_orange", "flower3_purple", "flower3_red", "flower3_white", "flower3_yellow",
			"flower5_black", "flower5_blue", "flower5_green", "flower5_orange", "flower5_purple", "flower5_red", "flower5_white", "flower5_yellow",
		};

		// And some from Tilation.
		const std::unordered_set<std::string> credit_override_tilation{
			"floor", "plant_pot1", "plant_pot2", "plant_pot3", "pot", "wooden_wall",
		};

		std::filesystem::path base = "split";

		std::filesystem::create_directory(base);

		// Necessary for Gdk::Pixbuf::getImage to work.
		auto app = Game3::App::create();

		for (const auto &[id, item]: game->registry<ItemRegistry>()) {
			std::filesystem::path dir = base / id.getPostPath();
			std::filesystem::create_directory(dir);

			auto image = item->getImage(*game, ItemStack(*game, item));

			// The images returned by Item::getImage are scaled up by a factor of 8.
			constexpr static int scale = 8;
			image = image->scale_simple(image->get_width() / scale, image->get_height() / scale, Gdk::InterpType::NEAREST);

			guint length{};
			guint8 *pixels = image->get_pixels(length);

			double root = std::sqrt(length / 4.);
			if (fractional(root) != 0.)
				throw std::runtime_error("Pixel length not a perfect square: " + std::to_string(length));

			const int dimension = static_cast<int>(root);

			auto raw = std::make_unique<uint8_t[]>(dimension * dimension * 4);

			for (int y = 0; y < dimension; ++y) {
				for (int x = 0; x < dimension; ++x) {
					size_t offset = (y * dimension + x) * 4;
					std::memcpy(&raw[offset], &pixels[offset], 4);
				}
			}

			std::filesystem::path png = dir / "item.png";
			if (!stbi_write_png(png.c_str(), dimension, dimension, 4, raw.get(), dimension * 4))
				throw std::runtime_error("Couldn't write png to " + png.string());

			nlohmann::json meta{
				{"size", std::make_pair(dimension, dimension)},
				{"id",   id},
			};

			const std::string end = id.getPostPath();

			if (credit_override_tilation.contains(end)) {
				meta["credit"] = "https://tilation.itch.io/16x16-small-indoor-tileset";
			} else if (credit_override_fayabella.contains(end)) {
				meta["credit"] = "Fayabella";
			} else if (credit_override_vadim.contains(end)) {
				meta["credit"] = "Vadim";
			} else {
				const Identifier texture_id = item->getTexture(ItemStack(*game, item))->identifier;
				if (auto iter = credits.find(texture_id); iter != credits.end())
					meta["credit"] = iter->second;
				else
					WARN("No credit for " << id << " in texture " << texture_id);
			}

			std::ofstream(dir / "item.json") << meta.dump();
		}
	}
}
