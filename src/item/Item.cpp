#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "item/Item.h"
#include "realm/Realm.h"
#include "registry/Registries.h"

namespace Game3 {

// ItemTexture

	ItemTexture::operator bool() const {
		return x != -1 && y != -1 && texture.lock() && width != -1 && height != -1;
	}

	void ItemTexture::fromJSON(Game &game, const nlohmann::json &json, ItemTexture &item_texture) {
		item_texture.textureName = json.at(0);
		item_texture.texture = game.registry<TextureRegistry>().at(item_texture.textureName);
		item_texture.x = json.at(1);
		item_texture.y = json.at(2);

		if (3 < json.size())
			item_texture.width = json.at(3);
		else
			item_texture.width = DEFAULT_WIDTH;

		if (4 < json.size())
			item_texture.height = json.at(4);
		else
			item_texture.height = DEFAULT_HEIGHT;
	}

	void to_json(nlohmann::json &json, const ItemTexture &item_texture) {
		json.push_back(item_texture.textureName);
		json.push_back(item_texture.x);
		json.push_back(item_texture.y);
		json.push_back(item_texture.width);
		json.push_back(item_texture.height);
	}

// Item

	Item::Item(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count):
		id(std::move(id_)), name(std::move(name_)), basePrice(base_price), maxCount(max_count) {}

	Glib::RefPtr<Gdk::Pixbuf> Item::getImage(const Game &game) {
		if (!cachedImage)
			cachedImage = makeImage(game);
		return cachedImage;
	}

	Glib::RefPtr<Gdk::Pixbuf> Item::makeImage(const Game &game) {
		auto item_texture = game.registry<ItemTextureRegistry>().at(id);
		auto &texture = *item_texture->texture.lock();
		texture.init();
		const int width  = item_texture->width;
		const int height = item_texture->height;
		const int channels = *texture.format == GL_RGBA? 4 : 3;
		const size_t row_size = channels * width;

		if (!rawImage) {
			rawImage = std::make_unique<uint8_t[]>(channels * width * height);
			uint8_t *raw_pointer = rawImage.get();
			uint8_t *texture_pointer = texture.data->get() + item_texture->y * *texture.width * channels + item_texture->x * channels;
			for (int row = 0; row < height; ++row) {
				std::memcpy(raw_pointer + row_size * row, texture_pointer, row_size);
				texture_pointer += channels * *texture.width;
			}
		}

		constexpr int doublings = 3;
		return Gdk::Pixbuf::create_from_data(rawImage.get(), Gdk::Colorspace::RGB, *texture.alpha, 8, width, height, row_size)
		       ->scale_simple(width << doublings, height << doublings, Gdk::InterpType::NEAREST);
	}

	void Item::getOffsets(const Game &game, Texture *&texture, float &x_offset, float &y_offset) {
		auto item_texture = game.registry<ItemTextureRegistry>().at(id);
		texture  = item_texture->texture.lock().get();
		x_offset = item_texture->x / 2.f;
		y_offset = item_texture->y / 2.f;
	}

	std::shared_ptr<Item> Item::addAttribute(ItemAttribute attribute) {
		attributes.insert(attribute);
		return shared_from_this();
	}

// ItemStack

	ItemStack::ItemStack(const Game &game, const ItemID &id, ItemCount count_):
		item(game.registry<ItemRegistry>().at(id)), count(count_) {}

	ItemStack::ItemStack(const Game &game, const ItemID &id, ItemCount count_, nlohmann::json data_):
		item(game.registry<ItemRegistry>().at(id)), count(count_), data(data_) {}

	bool ItemStack::canMerge(const ItemStack &other) const {
		return *item == *other.item && data == other.data;
	}

	Glib::RefPtr<Gdk::Pixbuf> ItemStack::getImage(const Game &game) {
		if (cachedImage)
			return cachedImage;

		if (item)
			return cachedImage = item->getImage(game);

		return {};
	}

	ItemStack ItemStack::withCount(ItemCount new_count) const {
		return {item, new_count};
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id, Durability durability) {
		ItemStack out(game, id, 1);
		out.data["maxDurability"] = durability;
		out.data["durability"]    = durability;
		return out;
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id) {
		return withDurability(game, id, *game.registry<DurabilityRegistry>().at(id));
	}

	bool ItemStack::reduceDurability(Durability amount) {
		if (!hasDurability())
			return false;
		return (data["durability"].at(0) = std::max(0, data["durability"].at(0).get<Durability>() - amount)) == 0;
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

		const std::pair<double, double> &pair = data.at("durability");
		return pair.first / pair.second;
	}

	void ItemStack::spawn(const std::shared_ptr<Realm> &realm, const Position &position) const {
		realm->spawn<ItemEntity>(position, *this);
	}

	void ItemStack::fromJSON(Game &game, const nlohmann::json &json, ItemStack &stack) {
		Identifier id = json.at(0);
		stack.item = game.registries.get<ItemRegistry>().at(id);
		stack.count = json.at(1);
		if (2 < json.size()) {
			const auto &extra = json.at(2);
			if (extra.is_string() && extra == "with_durability") {
				const Durability durability = *game.registry<DurabilityRegistry>().at(id);
				stack.data["durability"] = std::make_pair(durability, durability);
			} else {
				stack.data = extra;
			}
		}
	}

	ItemStack ItemStack::fromJSON(Game &game, const nlohmann::json &json) {
		ItemStack out;
		fromJSON(game, json, out);
		return out;
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->id;
		json[1] = stack.count;
		if (!stack.data.empty())
			json[2] = stack.data;
	}
}

std::ostream & operator<<(std::ostream &os, const Game3::ItemStack &stack) {
	return os << stack.item->name << " x " << stack.count;
}
