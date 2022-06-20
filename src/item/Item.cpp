#include "Texture.h"
#include "entity/ItemEntity.h"
#include "item/Furniture.h"
#include "item/Item.h"
#include "item/Sapling.h"
#include "item/Tool.h"
#include "realm/Realm.h"

namespace Game3 {
	static Texture textureShortsword  {"resources/items/shortsword.png"};
	static Texture textureConsumables {"resources/rpg/consumables.png"};
	static Texture texturePotions     {"resources/rpg/potions.png"};
	static Texture textureItems       {"resources/items/items.png"};
	static Texture texturePalisade    {"resources/items/palisade.png"};
	static Texture textureHouse       {"resources/house.png"};

	std::unordered_map<ItemID, ItemTexture> Item::itemTextures {
		{Item::SHORTSWORD,      {  0,   0, textureShortsword}},
		{Item::RED_POTION,      { 48, 176, texturePotions}},
		{Item::COINS,           {160, 112, textureItems}},
		{Item::IRON_ORE,        {128, 272, textureItems}},
		{Item::COPPER_ORE,      {160, 272, textureItems}},
		{Item::GOLD_ORE,        { 80, 272, textureItems}},
		{Item::DIAMOND_ORE,     { 96, 272, textureItems}},
		{Item::DIAMOND,         {  0, 128, textureItems}},
		{Item::COAL,            {192, 272, textureItems}},
		{Item::OIL,             {  0, 240, textureItems}},
		{Item::WOOD,            {208, 336, textureItems}},
		{Item::IRON_AXE,        { 16, 160, texturePalisade}},
		{Item::IRON_PICKAXE,    {128, 160, texturePalisade}},
		{Item::IRON_SHOVEL,     {208, 160, texturePalisade}},
		{Item::SAND,            { 16,   0, texturePalisade}},
		{Item::STONE,           {208, 320, textureItems}},
		{Item::IRON_BAR,        { 32, 192, textureItems}},
		{Item::SAPLING,         { 32,   0, texturePalisade}},
		{Item::GOLD_AXE,        { 32, 160, texturePalisade}},
		{Item::GOLD_PICKAXE,    {160, 160, texturePalisade}},
		{Item::GOLD_SHOVEL,     {224, 160, texturePalisade}},
		{Item::GOLD_BAR,        { 48, 192, textureItems}},
		{Item::DIAMOND_AXE,     { 48, 160, texturePalisade}},
		{Item::DIAMOND_PICKAXE, {176, 160, texturePalisade}},
		{Item::DIAMOND_SHOVEL,  {240, 160, texturePalisade}},
		{Item::WOODEN_WALL,     { 16, 144, textureHouse}},
		{Item::PLANK,           { 64, 224, textureItems}},
		{Item::CLAY,            { 48,   0, texturePalisade}},
		{Item::DIRT,            {128, 128, texturePalisade}},
		{Item::BRICK,           {256, 320, textureItems}},
	};

	std::unordered_map<ItemID, Durability> Item::durabilities {
		{Item::IRON_AXE,        128},
		{Item::IRON_PICKAXE,    128},
		{Item::IRON_SHOVEL,     128},
		{Item::GOLD_AXE,         64},
		{Item::GOLD_PICKAXE,     64},
		{Item::GOLD_SHOVEL,      64},
		{Item::DIAMOND_AXE,     512},
		{Item::DIAMOND_PICKAXE, 512},
		{Item::DIAMOND_SHOVEL,  512},
	};

	std::map<ItemID, std::shared_ptr<Item>> Item::items {
		{Item::SHORTSWORD,      std::make_shared<Item>     (Item::SHORTSWORD,      "Shortsword",      100,  1)},
		{Item::RED_POTION,      std::make_shared<Item>     (Item::RED_POTION,      "Red Potion",       20,  8)},
		{Item::COINS,           std::make_shared<Item>     (Item::COINS,           "Gold",              1, 1'000'000)},
		{Item::IRON_ORE,        std::make_shared<Item>     (Item::IRON_ORE,        "Iron Ore",         10, 64)},
		{Item::COPPER_ORE,      std::make_shared<Item>     (Item::COPPER_ORE,      "Copper Ore",        8, 64)},
		{Item::GOLD_ORE,        std::make_shared<Item>     (Item::GOLD_ORE,        "Gold Ore",         20, 64)},
		{Item::DIAMOND_ORE,     std::make_shared<Item>     (Item::DIAMOND_ORE,     "Diamond Ore",      80, 64)},
		{Item::DIAMOND,         std::make_shared<Item>     (Item::DIAMOND,         "Diamond",         100, 64)},
		{Item::COAL,            std::make_shared<Item>     (Item::COAL,            "Coal",              5, 64)},
		{Item::OIL,             std::make_shared<Item>     (Item::OIL,             "Oil",              15, 64)},
		{Item::WOOD,            std::make_shared<Item>     (Item::WOOD,            "Wood",              3, 64)},
		{Item::IRON_AXE,        std::make_shared<Tool>     (Item::IRON_AXE,        "Iron Axe",        150, 3.f, ItemAttribute::Axe)},
		{Item::IRON_PICKAXE,    std::make_shared<Tool>     (Item::IRON_PICKAXE,    "Iron Pickaxe",    150, 3.f, ItemAttribute::Pickaxe)},
		{Item::IRON_SHOVEL,     std::make_shared<Tool>     (Item::IRON_SHOVEL,     "Iron Shovel",     120, 3.f, ItemAttribute::Shovel)},
		{Item::SAND,            std::make_shared<Item>     (Item::SAND,            "Sand",              1, 64)},
		{Item::STONE,           std::make_shared<Item>     (Item::STONE,           "Stone",             1, 64)},
		{Item::IRON_BAR,        std::make_shared<Item>     (Item::IRON_BAR,        "Iron Bar",         16, 64)},
		{Item::SAPLING,         std::make_shared<Sapling>  (Item::SAPLING,         "Sapling",           2, 64)},
		{Item::GOLD_AXE,        std::make_shared<Tool>     (Item::GOLD_AXE,        "Gold Axe",        400, 0.75f, ItemAttribute::Axe)},
		{Item::GOLD_PICKAXE,    std::make_shared<Tool>     (Item::GOLD_PICKAXE,    "Gold Pickaxe",    400, 0.75f, ItemAttribute::Pickaxe)},
		{Item::GOLD_SHOVEL,     std::make_shared<Tool>     (Item::GOLD_SHOVEL,     "Gold Shovel",     300, 0.75f, ItemAttribute::Shovel)},
		{Item::GOLD_BAR,        std::make_shared<Item>     (Item::GOLD_BAR,        "Gold Bar",         45, 64)},
		{Item::DIAMOND_AXE,     std::make_shared<Tool>     (Item::DIAMOND_AXE,     "Diamond Axe",     900, 1.f, ItemAttribute::Axe)},
		{Item::DIAMOND_PICKAXE, std::make_shared<Tool>     (Item::DIAMOND_PICKAXE, "Diamond Pickaxe", 900, 1.f, ItemAttribute::Pickaxe)},
		{Item::DIAMOND_SHOVEL,  std::make_shared<Tool>     (Item::DIAMOND_SHOVEL,  "Diamond Shovel",  700, 1.f, ItemAttribute::Shovel)},
		{Item::WOODEN_WALL,     std::make_shared<Furniture>(Item::WOODEN_WALL,     "Wooden Wall",       9, 64)},
		{Item::PLANK,           std::make_shared<Item>     (Item::PLANK,           "Plank",             4, 64)},
		{Item::CLAY,            std::make_shared<Item>     (Item::CLAY,            "Clay",              2, 64)},
		{Item::DIRT,            std::make_shared<Item>     (Item::DIRT,            "Dirt",              1, 64)},
		{Item::BRICK,           std::make_shared<Item>     (Item::BRICK,           "Brick",             3, 64)},
	};

	Glib::RefPtr<Gdk::Pixbuf> Item::getImage() {
		auto &item_texture = Item::itemTextures.at(id);
		auto &texture = *item_texture.texture;
		texture.init();
		const auto width  = item_texture.width;
		const auto height = item_texture.height;
		const int channels = texture.format == GL_RGBA? 4 : 3;
		const size_t row_size = channels * width;

		if (!rawImage) {
			rawImage = std::make_unique<uint8_t[]>(channels * width * height);
			uint8_t *raw_pointer = rawImage.get();
			uint8_t *texture_pointer = texture.data.get() + item_texture.y * texture.width * channels + item_texture.x * channels;
			for (auto row = 0; row < height; ++row) {
				std::memcpy(raw_pointer + row_size * row, texture_pointer, row_size);
				texture_pointer += channels * texture.width;
			}
		}

		constexpr int doublings = 3;
		return Gdk::Pixbuf::create_from_data(rawImage.get(), Gdk::Colorspace::RGB, texture.alpha, 8, width, height, row_size)
		       ->scale_simple(width << doublings, height << doublings, Gdk::InterpType::NEAREST);
	}

	std::shared_ptr<Item> Item::addAttribute(ItemAttribute attribute) {
		attributes.insert(attribute);
		return shared_from_this();
	}

	bool ItemStack::canMerge(const ItemStack &other) const {
		return *item == *other.item && data == other.data;
	}

	Glib::RefPtr<Gdk::Pixbuf> ItemStack::getImage() {
		if (cachedImage)
			return cachedImage;

		if (item)
			return cachedImage = item->getImage();

		return {};
	}

	ItemStack ItemStack::withCount(ItemCount new_count) const {
		return {item, new_count};
	}

	ItemStack ItemStack::withDurability(ItemID id, Durability durability) {
		ItemStack out(id, 1);
		out.data["maxDurability"] = durability;
		out.data["durability"]    = durability;
		return out;
	}

	ItemStack ItemStack::withDurability(ItemID id) {
		return withDurability(id, Item::durabilities.at(id));
	}

	bool ItemStack::reduceDurability(Durability amount) {
		if (!hasDurability())
			return false;
		return (data["durability"] = std::max(0, data["durability"].get<Durability>() - amount)) == 0;
	}

	bool ItemStack::has(ItemAttribute attribute) const {
		return item->attributes.contains(attribute);
	}

	bool ItemStack::hasDurability() const {
		return data.contains("durability");
	}

	double ItemStack::getDurabilityFraction() const {
		if (!hasDurability())
			return 1.;

		return data.at("durability").get<double>() / data.at("maxDurability").get<double>();
	}

	void ItemStack::spawn(const std::shared_ptr<Realm> &realm, const Position &position) const {
		realm->spawn<ItemEntity>(position, *this);
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->id;
		json[1] = stack.count;
		if (!stack.data.empty())
			json[2] = stack.data;
	}

	void from_json(const nlohmann::json &json, ItemStack &stack) {
		stack.item = Item::items.at(json.at(0));
		stack.count = json.at(1);
		if (2 < json.size())
			stack.data = json.at(2);
	}
}
