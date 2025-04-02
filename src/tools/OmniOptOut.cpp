#include "util/Log.h"
#include "container/StringSet.h"
#include "lib/JSON.h"
#include "util/FS.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unordered_set>

namespace Game3 {
	namespace {
		bool isMatch(const std::filesystem::path &path, boost::json::value &json, bool &is_crop) {
			const std::string filename = path.filename().string();
			bool good = false;

			const static std::vector<std::string> crop_matches{
				"eggplant", "corn", "potato", "strawberry", "cotton", "onion", "bean", "wheat", "cabbage", "tomato", "pumpkin", "carrot"
			};

			for (const std::string &crop_matchable: crop_matches) {
				if (filename.find(crop_matchable) != std::string::npos) {
					is_crop = true;
					good = true;
					break;
				}
			}

			const static std::vector<std::string> partial_matches{
				"carpet", "plant"
			};

			for (const std::string &partial_matchable: partial_matches) {
				if (filename.find(partial_matchable) != std::string::npos) {
					good = true;
					break;
				}
			}

			json = boost::json::parse(readFile(path / "tile.json"));

			if (good) {
				return true;
			}

			const static StringSet category_matches{
				"base:category/plants"
			};

			if (auto *categories = json.as_object().if_contains("categories")) {
				for (const auto &category: categories->as_array()) {
					if (category_matches.contains(static_cast<std::string_view>(category.as_string()))) {
						return true;
					}
				}
			}

			return false;
		}
	}

	void omniOptOut() {
		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator("resources/tileset")) {
			if (!entry.is_directory()) {
				continue;
			}

			const std::filesystem::path path = entry.path();
			boost::json::value json;
			bool is_crop = false;
			if (!isMatch(path, json, is_crop)) {
				continue;
			}

			auto *object = json.if_object();
			if (!object) {
				continue;
			}

			auto *categories = object->if_contains("categories");
			if (!categories) {
				continue;
			}

			StringSet categories_set = boost::json::value_to<StringSet>(*categories);

			categories_set.insert("base:category/no_omni");
			if (is_crop) {
				categories_set.insert("base:category/crop");
			}

			*categories = boost::json::value_from(categories_set);
			std::ofstream ofs(path / "tile.json");

			boost::json::serializer serializer;
			serializer.reset(&json);
			char buffer[512];
			while (!serializer.done()) {
				ofs << serializer.read(buffer);
			}

			SUCCESS("Patched {}", path.filename().string());
		}
	}
}
