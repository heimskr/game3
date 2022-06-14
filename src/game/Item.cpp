#include "Texture.h"
#include "game/Item.h"

namespace Game3 {
	static Texture textureShortsword  {"resources/items/shortsword.png"};
	static Texture textureConsumables {"resources/rpg/consumables.png"};
	static Texture texturePotions     {"resources/rpg/potions.png"};
	static Texture textureItems       {"resources/items/items.png"};
	static Texture texturePalisade    {"resources/items/palisade.png"};

	std::unordered_map<ItemID, ItemTexture> Item::itemTextures {
		{Item::SHORTSWORD,  {  0,   0, textureShortsword}},
		{Item::RED_POTION,  { 48, 176, texturePotions}},
		{Item::COINS,       {160, 112, textureItems}},
		{Item::IRON_ORE,    {128, 272, textureItems}},
		{Item::COPPER_ORE,  {160, 272, textureItems}},
		{Item::GOLD_ORE,    { 80, 272, textureItems}},
		{Item::DIAMOND_ORE, { 96, 272, textureItems}},
		{Item::DIAMOND,     {  0, 128, textureItems}},
		{Item::COAL,        {192, 272, textureItems}},
		{Item::OIL,         {  0, 240, textureItems}},
		{Item::WOOD,        {208, 336, textureItems}},
		{Item::IRON_AXE,    { 32,  96, texturePalisade}},
	};

	std::map<ItemID, std::shared_ptr<Item>> Item::items {
		{Item::SHORTSWORD,  std::make_shared<Item>(Item::SHORTSWORD,  "Shortsword", 100,  1)},
		{Item::RED_POTION,  std::make_shared<Item>(Item::RED_POTION,  "Red Potion",  20,  8)},
		{Item::COINS,       std::make_shared<Item>(Item::COINS,       "Gold",         1, 1'000'000)},
		{Item::IRON_ORE,    std::make_shared<Item>(Item::IRON_ORE,    "Iron Ore",    10, 64)},
		{Item::COPPER_ORE,  std::make_shared<Item>(Item::COPPER_ORE,  "Copper Ore",   8, 64)},
		{Item::GOLD_ORE,    std::make_shared<Item>(Item::GOLD_ORE,    "Gold Ore",    20, 64)},
		{Item::DIAMOND_ORE, std::make_shared<Item>(Item::DIAMOND_ORE, "Diamond Ore", 80, 64)},
		{Item::DIAMOND,     std::make_shared<Item>(Item::DIAMOND,     "Diamond",    100, 64)},
		{Item::COAL,        std::make_shared<Item>(Item::COAL,        "Coal",         5, 64)},
		{Item::OIL,         std::make_shared<Item>(Item::OIL,         "Oil",         15, 64)},
		{Item::WOOD,        std::make_shared<Item>(Item::WOOD,        "Wood",         3, 64)},
		{Item::IRON_AXE,    std::make_shared<Item>(Item::IRON_AXE,    "Iron Axe",    50,  1)},
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

	bool ItemStack::reduceDurability(Durability amount) {
		if (!data.contains("durability"))
			return false;
		return (data["durability"] = std::max(0, data["durability"].get<Durability>() - amount)) == 0;
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
