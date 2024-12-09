#include "Log.h"
#include "util/FS.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <unordered_set>

#include <boost/json.hpp>

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

			if (good)
				return true;

			const static std::unordered_set<std::string> category_matches{
				"base:category/plants"
			};

			if (auto iter = json.find("categories"); iter != json.end())
				for (const boost::json::value &category: *iter)
					if (category_matches.contains(category.get<std::string>()))
						return true;

			return false;
		}
	}

	void omniOptOut() {
		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator("resources/tileset")) {
			if (!entry.is_directory())
				continue;

			const std::filesystem::path path = entry.path();
			boost::json::value json;
			bool is_crop = false;
			if (!isMatch(path, json, is_crop))
				continue;

			boost::json::value &categories = json["categories"];

			std::set<std::string> categories_set;
			if (!categories.is_null())
				categories_set = categories;

			categories_set.insert("base:category/no_omni");
			if (is_crop)
				categories_set.insert("base:category/crop");

			categories = std::move(categories_set);
			std::ofstream ofs(path / "tile.json");
			ofs << json.dump();

			SUCCESS("Patched {}", path.filename().string());
		}
	}
}
