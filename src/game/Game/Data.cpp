#include "data/ConsumptionRule.h"
#include "data/ProductionRule.h"
#include "data/RegisterableIdentifier.h"
#include "data/SoundSet.h"
#include "entity/Entity.h"
#include "game/Crop.h"
#include "game/Game.h"
#include "graph/Graph.h"
#include "lib/JSON.h"
#include "recipe/BiomassLiquefierRecipe.h"
#include "recipe/CombinerRecipe.h"
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
		std::map<std::string, std::vector<std::string>> categories;
		std::map<std::string, boost::json::value> jsons;

		auto add_dependencies = [&](const boost::json::value &json) {
			std::string name(json.at("name").as_string());

			bool created{};
			auto &node = dependencies.get(name, created);
			if (created) {
				node.data = {name, false};
			}

			for (const auto &dependency_json: json.at("dependencies").as_array()) {
				const std::string order(dependency_json.at(0).as_string());
				const bool is_after = order == "after";
				if (!is_after && order != "before") {
					throw std::runtime_error("Couldn't load JSON: invalid order \"" + order + '"');
				}

				const std::string specifier(dependency_json.at(1).as_string());
				const bool is_type = specifier == "type";
				if (!is_type && specifier != "name") {
					throw std::runtime_error("Couldn't load JSON: invalid specifier \"" + specifier + '"');
				}

				const std::string id(dependency_json.at(2).as_string());
				const bool is_category = id.find(':') != std::string::npos;

				auto &other_node = dependencies.get(id, created);
				if (created) {
					other_node.data = {id, is_category};
				}

				if (is_after) {
					other_node.link(node);
				} else {
					node.link(other_node);
				}
			}
		};

		std::function<void(const std::filesystem::path &)> traverse = [&](const auto &dir) {
			for (const auto &entry: std::filesystem::directory_iterator(dir)) {
				if (entry.is_directory()) {
					traverse(entry.path());
				} else if (entry.is_regular_file()) {
					if (auto path = entry.path(); path.extension() == ".json") {
						json_paths.push_back(std::move(path));
					}
				}
			}
		};

		traverse(dir);

		for (const auto &path: json_paths) {
			std::ifstream ifs(path);
			std::stringstream ss;
			ss << ifs.rdbuf();
			boost::json::value json = boost::json::parse(ss.str());
			add_dependencies(json);
			std::string name(json.at("name").as_string());
			for (const boost::json::value &item: json.at("data").as_array()) {
				categories[std::string(item.at(0).as_string())].push_back(name);
			}
			jsons.emplace(std::move(name), std::move(json));
		}

		for (const auto &[category, names]: categories) {
			auto *category_node = dependencies.maybe(category);
			if (category_node == nullptr) {
				continue; // ???
			}

			for (const std::string &name: names) {
				auto &name_node = dependencies[name];

				for (const auto &in: category_node->getIn()) {
					in->link(name_node);
				}

				for (const auto &out: category_node->getOut()) {
					name_node.link(*out);
				}
			}
		}

		for (const auto &[category, names]: categories) {
			if (dependencies.hasLabel(category)) {
				dependencies -= category;
			}
		}

		for (const auto &node: dependencies.topoSort()) {
			assert(!node->data.isCategory);
			for (const boost::json::value &json: jsons.at(node->data.name).at("data").as_array()) {
				loadData(json);
			}
		}
	}

	void Game::loadData(const boost::json::value &json) {
		Identifier type = boost::json::value_to<Identifier>(json.at(0));

		// TODO: make a map of handlers for different types instead of if-elsing here
		if (type == "base:entity_texture_map") {

			auto &textures = registry<EntityTextureRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				textures.add(Identifier(key), EntityTexture(Identifier(key), boost::json::value_to<Identifier>(value.at(0)), getNumber<uint8_t>(value.at(1))));
			}

		} else if (type == "base:ore_map") {

			GamePtr self = shared_from_this();
			auto &ores = registry<OreRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				Identifier ore_key(key);
				ItemStackPtr ore_stack = boost::json::value_to<ItemStackPtr>(value.at(0), self);
				Identifier tilename = boost::json::value_to<Identifier>(value.at(1));
				Identifier regen_tilename = boost::json::value_to<Identifier>(value.at(2));
				float tooldown_multiplier = getNumber<float>(value.at(3));
				uint32_t max_uses = getNumber<uint32_t>(value.at(4));
				float cooldown = getNumber<float>(value.at(5));
				Ore ore(ore_key, std::move(ore_stack), std::move(tilename), std::move(regen_tilename), tooldown_multiplier, max_uses, cooldown);
				ores.add(std::move(ore_key), std::move(ore));
			}

		} else if (type == "base:realm_details_map") {

			auto &details = registry<RealmDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				Identifier realm_key(key);
				RealmDetails realm_details(realm_key, boost::json::value_to<Identifier>(value.at("tileset")));
				details.add(std::move(realm_key), std::move(realm_details));
			}

		} else if (type == "base:texture_map") {

			auto &textures = registry<TextureRegistry>();
			if (getSide() == Side::Client) {
				for (const auto &[key, value]: json.at(1).as_object()) {
					Identifier texture_key(key);
					const auto &array = value.as_array();
					if (array.size() < 1 || 3 < array.size()) {
						throw std::invalid_argument("Expected Texture JSON size to be 1, 2 or 3, not " + std::to_string(array.size()));
					}

					std::filesystem::path path(getString(array[0]));
					TexturePtr texture;

					if (array.size() == 1) {
						texture = std::make_shared<Texture>(texture_key, std::move(path));
					} else if (array.size() == 2) {
						texture = std::make_shared<Texture>(texture_key, std::move(path), value.at(1).as_bool());
					} else {
						texture = std::make_shared<Texture>(texture_key, std::move(path), value.at(1).as_bool(), getNumber<int>(value.at(2)));
					}

					texture->init();
					textures.add(std::move(texture_key), std::move(texture));
				}
			} else {
				for (const auto &[key, value]: json.at(1).as_object()) {
					textures.add(Identifier(key), std::make_shared<Texture>(Identifier(key)));
				}
			}

		} else if (type == "base:tileset") {

			Identifier identifier = boost::json::value_to<Identifier>(json.at(1));
			std::filesystem::path base_dir(std::string_view(json.at(2).as_string()));
			auto &tilesets = registry<TilesetRegistry>();
			tilesets.add(identifier, tileStitcher(base_dir, identifier, getSide()));

		} else if (type == "base:itemset") {

			if (getSide() == Side::Client) {
				Identifier identifier = boost::json::value_to<Identifier>(json.at(1));
				std::filesystem::path base_dir(std::string_view(json.at(2).as_string()));
				auto &itemsets = registry<ItemSetRegistry>();
				itemsets.add(identifier, itemStitcher(&registry<ItemTextureRegistry>(), &registry<ResourceRegistry>(), base_dir, identifier));
			}

		} else if (type == "base:sound_source") {

			addSounds(std::string_view(json.at(1).as_string()));

		} else if (type == "base:recipe_list") {

			for (const auto &recipe_json: json.at(1).as_array()) {
				addRecipe(recipe_json);
			}

		} else if (type == "base:dissolver_map") {

			GamePtr self = shared_from_this();
			auto &recipes = registry<DissolverRecipeRegistry>();
			for (const auto &[input, result_json]: json.at(1).as_object()) {
				const Identifier identifier(input);
				recipes.add(identifier, DissolverRecipe(identifier, ItemStack::create(self, identifier, 1), result_json));
			}

		} else if (type == "base:biomass_liquefier_map") {

			GamePtr self = shared_from_this();
			auto &recipes = registry<BiomassLiquefierRecipeRegistry>();
			for (const auto &[input, result_json]: json.at(1).as_object()) {
				const Identifier identifier(input);
				recipes.add(identifier, BiomassLiquefierRecipe(ItemStack::create(self, identifier, 1), boost::json::value_to<FluidAmount>(result_json)));
			}

		} else if (type == "base:combiner_map") {

			GamePtr self = shared_from_this();
			auto &recipes = registry<CombinerRecipeRegistry>();
			for (const auto &[input, input_json]: json.at(1).as_object()) {
				const Identifier identifier(input);
				recipes.add(identifier, CombinerRecipe(identifier, self, input_json));
			}

		} else if (type == "base:fluid_list") {

			auto &fluids = registry<FluidRegistry>();
			for (const boost::json::value &pair: json.at(1).as_array()) {
				Identifier fluid_name = boost::json::value_to<Identifier>(pair.at(0));
				boost::json::object value = pair.at(1).as_object();
				std::string name(value.at("name").as_string());
				Identifier tileset_name = boost::json::value_to<Identifier>(value.at("tileset"));
				Identifier tilename = boost::json::value_to<Identifier>(value.at("tilename"));
				Color color = boost::json::value_to<Color>(value.at("color"));
				std::optional<Fluid> fluid;
				if (auto iter = value.find("flask"); iter != value.end()) {
					Identifier flask = boost::json::value_to<Identifier>(iter->value());
					fluid.emplace(fluid_name, std::move(name), std::move(tileset_name), std::move(tilename), color, std::move(flask));
				} else {
					fluid.emplace(fluid_name, std::move(name), std::move(tileset_name), std::move(tilename), color);
				}
				fluids.add(std::move(fluid_name), std::move(*fluid));
			}

		} else if (type == "base:production_list") {

			GamePtr self = shared_from_this();
			auto &rules = registry<ProductionRuleRegistry>();
			for (const boost::json::value &rule: json.at(1).as_array()) {
				rules.add(self, rule);
			}

		} else if (type == "base:consumption_list") {

			GamePtr self = shared_from_this();
			auto &rules = registry<ConsumptionRuleRegistry>();
			for (const boost::json::value &rule: json.at(1).as_array()) {
				rules.add(self, rule);
			}

		} else if (type == "base:crop_map") {

			GamePtr self = shared_from_this();
			auto &crops = registry<CropRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				crops.add(Identifier(key), Crop(Identifier(key), self, value));
			}

		} else if (type == "base:attribute_exemplar_map") {

			auto &exemplars = registry<AttributeExemplarRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				exemplars.add(Identifier(key), RegisterableIdentifier(Identifier(key), boost::json::value_to<Identifier>(value)));
			}

		} else if (type == "base:sound_set_map") {

			auto &sound_sets = registry<SoundSetRegistry>();
			for (const auto &[key, value]: json.at(1).as_object()) {
				SoundSet::Set set;
				float pitch_variance = 1;
				for (const boost::json::value &subvalue: value.as_array()) {
					if (subvalue.is_string()) {
						set.emplace(boost::json::value_to<Identifier>(subvalue));
					} else {
						pitch_variance = getNumber<float>(subvalue);
					}
				}
				sound_sets.add(Identifier(key), std::make_shared<SoundSet>(Identifier(key), std::move(set), pitch_variance));
			}

		} else if (type.getPathStart() == "ignore") {

			// For old data that isn't ready to be removed yet.

		} else {
			throw std::runtime_error("Unknown data bundle type: " + type.str());
		}
	}
}
