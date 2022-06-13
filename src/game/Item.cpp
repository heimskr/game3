#include "Texture.h"
#include "game/Item.h"

namespace Game3 {
	static Texture textureShortsword  {"resources/items/shortsword.png"};
	static Texture textureConsumables {"resources/rpg/consumables.png"};
	static Texture texturePotions     {"resources/rpg/potions.png"};
	static Texture textureItems       {"resources/items/items.png"};

	std::unordered_map<ItemID, ItemTexture> Item::itemTextures {
		{Item::SHORTSWORD, {0, 0, textureShortsword}},
		{Item::RED_POTION, {48, 176, texturePotions}},
		{Item::COINS,      {160, 112, textureItems}},
	};

	std::map<ItemID, std::shared_ptr<Item>> Item::items {
		{Item::SHORTSWORD, std::make_shared<Item>(Item::SHORTSWORD, "Shortsword", 100, 1)},
		{Item::RED_POTION, std::make_shared<Item>(Item::RED_POTION, "Red Potion", 20, 8)},
		{Item::COINS,      std::make_shared<Item>(Item::COINS, "Gold", 1, 1'000'000)},
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
		// TODO: update when items can store data.
		return item->id == other.item->id;
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

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->id;
		json[1] = stack.count;
	}

	void from_json(const nlohmann::json &json, ItemStack &stack) {
		stack.item = Item::items.at(json.at(0));
		stack.count = json.at(1);
	}
}
