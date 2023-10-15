#include "game/Crop.h"
#include "game/Game.h"
#include "graph/Graph.h"
#include "recipe/DissolverRecipe.h"
#include "tileentity/OreDeposit.h"
#include "tools/ItemStitcher.h"
#include "tools/TileStitcher.h"

namespace Game3 {
	struct DependencyNode {
		std::string name;
		bool isCategory;
	};

	void Game::traverseData(const std::filesystem::path &dir) {
		std::vector<std::filesystem::path> json_paths;
		// A -> B means A is loaded before B.
		Graph<DependencyNode> dependencies;
		// Maps data types like "base:crop_map" to vectors of names of data files that contain instances of them.
		std::unordered_map<std::string, std::vector<std::string>> categories;
		std::unordered_map<std::string, nlohmann::json> jsons;

		auto add_dependencies = [&](const nlohmann::json &json) {
			const std::string name = json.at("name");

			bool created{};
			auto &node = dependencies.get(name, created);
			if (created)
				node.data = {name, false};

			for (const auto &dependency_json: json.at("dependencies")) {
				const std::string order = dependency_json.at(0);
				const bool is_after = order == "after";
				if (!is_after && order != "before")
					throw std::runtime_error("Couldn't load JSON: invalid order \"" + order + '"');

				const std::string specifier = dependency_json.at(1);
				const bool is_type = specifier == "type";
				if (!is_type && specifier != "name")
					throw std::runtime_error("Couldn't load JSON: invalid specifier \"" + specifier + '"');

				const std::string id = dependency_json.at(2);
				const bool is_category = id.find(':') != std::string::npos;

				auto &other_node = dependencies.get(id, created);
				if (created)
					other_node.data = {id, is_category};

				if (is_after)
					other_node.link(node);
				else
					node.link(other_node);
			}
		};

		std::function<void(const std::filesystem::path &)> traverse = [&](const auto &dir) {
			for (const auto &entry: std::filesystem::directory_iterator(dir)) {
				if (entry.is_directory()) {
					traverse(entry.path());
				} else if (entry.is_regular_file()) {
					if (auto path = entry.path(); path.extension() == ".json")
						json_paths.push_back(std::move(path));
				}
			}
		};

		traverse(dir);

		for (const auto &path: json_paths) {
			std::ifstream ifs(path);
			std::stringstream ss;
			ss << ifs.rdbuf();
			nlohmann::json json = nlohmann::json::parse(ss.str());
			add_dependencies(json);
			std::string name = json.at("name");
			for (const nlohmann::json &item: json.at("data"))
				categories[item.at(0)].push_back(name);
			jsons.emplace(std::move(name), std::move(json));
		}

		for (const auto &[category, names]: categories) {
			auto *category_node = dependencies.maybe(category);
			if (category_node == nullptr)
				continue; // ???

			for (const std::string &name: names) {
				auto &name_node = dependencies[name];

				for (const auto &in: category_node->getIn())
					in->link(name_node);

				for (const auto &out: category_node->getOut())
					name_node.link(*out);
			}

		}

		for (const auto &[category, names]: categories)
			if (dependencies.hasLabel(category))
				dependencies -= category;

		for (const auto &node: dependencies.topoSort()) {
			assert(!node->data.isCategory);
			for (const nlohmann::json &json: jsons.at(node->data.name).at("data"))
				loadData(json);
		}
	}

	void Game::loadData(const nlohmann::json &json) {
		Identifier type = json.at(0);

		// TODO: make a map of handlers for different types instead of if-elsing here
		if (type == "base:entity_texture_map") {

			auto &textures = registry<EntityTextureRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				textures.add(Identifier(key), EntityTexture(Identifier(key), value.at(0), value.at(1)));

		} else if (type == "base:ore_map") {

			auto &ores = registry<OreRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				ores.add(Identifier(key), Ore(Identifier(key), ItemStack::fromJSON(*this, value.at(0)), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5)));

		} else if (type == "base:realm_details_map") {

			auto &details = registry<RealmDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), RealmDetails(Identifier(key), value.at("tileset")));

		} else if (type == "base:texture_map") {

			auto &textures = registry<TextureRegistry>();
			for (const auto &[key, value]: json.at(1).items()) {
				if (value.size() == 1)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0)))->init();
				else if (value.size() == 2)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1)))->init();
				else if (value.size() == 3)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1), value.at(2)))->init();
				else
					throw std::invalid_argument("Expected Texture JSON size to be 1, 2 or 3, not " + std::to_string(value.size()));
			}

		} else if (type == "base:tileset") {

			Identifier identifier = json.at(1);
			std::filesystem::path base_dir = json.at(2);
			auto &tilesets = registry<TilesetRegistry>();
			tilesets.add(identifier, tileStitcher(base_dir, identifier));

		} else if (type == "base:itemset") {

			Identifier identifier = json.at(1);
			std::filesystem::path base_dir = json.at(2);
			auto &itemsets = registry<ItemSetRegistry>();
			itemsets.add(identifier, itemStitcher(&registry<ItemTextureRegistry>(), base_dir, identifier));

		} else if (type == "base:recipe_list") {

			for (const auto &recipe_json: json.at(1))
				addRecipe(recipe_json);

		} else if (type == "base:dissolver_map") {

			auto &dissolver_recipes = registry<DissolverRecipeRegistry>();
			for (const auto &[input, result_json]: json.at(1).items()) {
				const Identifier identifier(input);
				dissolver_recipes.add(identifier, DissolverRecipe(identifier, ItemStack(*this, identifier, 1), result_json));
			}

		} else if (type == "base:fluid_list") {

			auto &fluids = registry<FluidRegistry>();
			for (const auto &pair: json.at(1)) {
				const Identifier fluid_name = pair.at(0);
				const nlohmann::json value = pair.at(1);
				if (auto iter = value.find("flask"); iter != value.end())
					fluids.add(fluid_name, Fluid(fluid_name, value.at("name"), value.at("tileset"), value.at("tilename"), *iter));
				else
					fluids.add(fluid_name, Fluid(fluid_name, value.at("name"), value.at("tileset"), value.at("tilename")));
			}

		} else if (type == "base:crop_map") {

			auto &crops = registry<CropRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				crops.add(Identifier(key), Crop(Identifier(key), *this, value));

		} else if (type.getPathStart() == "ignore") {

			// For old data that isn't ready to be removed yet.

		} else
			throw std::runtime_error("Unknown data file type: " + type.str());
	}
}
