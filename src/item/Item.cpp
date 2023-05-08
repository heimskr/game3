#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "item/HasMaxDurability.h"
#include "item/Item.h"
#include "realm/Realm.h"
#include "registry/Registries.h"

namespace Game3 {

// ItemTexture

	ItemTexture::ItemTexture(Identifier identifier_, Identifier texture_name, int x_, int y_, int width_, int height_):
		NamedRegisterable(std::move(identifier_)),
		x(x_),
		y(y_),
		textureName(std::move(texture_name)),
		width(width_),
		height(height_) {}

	std::shared_ptr<Texture> ItemTexture::getTexture(const Game &game) {
		if (auto locked = texture.lock())
			return locked;

		auto new_texture = game.registry<TextureRegistry>()[textureName];
		texture = new_texture;
		return new_texture;
	}

	ItemTexture::operator bool() const {
		return x != -1 && y != -1 && texture.lock() && width != -1 && height != -1;
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
		NamedRegisterable(std::move(id_)), name(std::move(name_)), basePrice(base_price), maxCount(max_count) {}

	Glib::RefPtr<Gdk::Pixbuf> Item::getImage(const Game &game) {
		if (!cachedImage)
			cachedImage = makeImage(game);
		return cachedImage;
	}

	Glib::RefPtr<Gdk::Pixbuf> Item::makeImage(const Game &game) {
		auto item_texture = game.registry<ItemTextureRegistry>().at(identifier);
		auto texture = item_texture->getTexture(game);
		texture->init();
		const int width  = item_texture->width;
		const int height = item_texture->height;
		const int channels = *texture->format == GL_RGBA? 4 : 3;
		const size_t row_size = channels * width;

		if (!rawImage) {
			rawImage = std::make_unique<uint8_t[]>(channels * width * height);
			uint8_t *raw_pointer = rawImage.get();
			uint8_t *texture_pointer = texture->data->get() + item_texture->y * *texture->width * channels + item_texture->x * channels;
			for (int row = 0; row < height; ++row) {
				std::memcpy(raw_pointer + row_size * row, texture_pointer, row_size);
				texture_pointer += channels * *texture->width;
			}
		}

		constexpr int doublings = 3;
		return Gdk::Pixbuf::create_from_data(rawImage.get(), Gdk::Colorspace::RGB, *texture->alpha, 8, width, height, row_size)
		       ->scale_simple(width << doublings, height << doublings, Gdk::InterpType::NEAREST);
	}

	void Item::getOffsets(const Game &game, std::shared_ptr<Texture> &texture, float &x_offset, float &y_offset) {
		auto item_texture = game.registry<ItemTextureRegistry>().at(identifier);
		texture  = item_texture->texture.lock();
		x_offset = item_texture->x / 2.f;
		y_offset = item_texture->y / 2.f;
	}

	std::shared_ptr<Item> Item::addAttribute(Identifier attribute) {
		attributes.insert(std::move(attribute));
		return shared_from_this();
	}

	std::shared_ptr<Texture> Item::getTexture(const Game &game) {
		if (cachedTexture)
			return cachedTexture;

		return cachedTexture = game.registry<ItemTextureRegistry>().at(identifier)->getTexture(game);
	}

	bool Item::use(Slot, ItemStack &, const Place &, Modifiers) {
		return false;
	}

// ItemStack

	ItemStack::ItemStack(const Game &game_):
		game(&game_) {}

	ItemStack::ItemStack(const Game &game_, std::shared_ptr<Item> item_, ItemCount count_):
	item(std::move(item_)), count(count_), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, std::shared_ptr<Item> item_, ItemCount count_, nlohmann::json data_):
	item(std::move(item_)), count(count_), data(std::move(data_)), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, const ItemID &id, ItemCount count_):
	item(game_.registry<ItemRegistry>().at(id)), count(count_), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, const ItemID &id, ItemCount count_, nlohmann::json data_):
	item(game_.registry<ItemRegistry>().at(id)), count(count_), data(std::move(data_)), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	bool ItemStack::canMerge(const ItemStack &other) const {
		return *item == *other.item && data == other.data;
	}

	Glib::RefPtr<Gdk::Pixbuf> ItemStack::getImage() {
		assert(game);
		return getImage(*game);
	}

	Glib::RefPtr<Gdk::Pixbuf> ItemStack::getImage(const Game &game_) {
		if (cachedImage)
			return cachedImage;

		if (item)
			return cachedImage = item->getImage(game_);

		return {};
	}

	ItemStack ItemStack::withCount(ItemCount new_count) const {
		assert(game);
		return {*game, item, new_count};
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id, Durability durability) {
		ItemStack out(game, id, 1);
		out.data["durability"] = std::make_pair(durability, durability);
		return out;
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id) {
		return withDurability(game, id, dynamic_cast<HasMaxDurability &>(*game.registry<ItemRegistry>()[id]).maxDurability);
	}

	bool ItemStack::reduceDurability(Durability amount) {
		if (!hasDurability())
			return false;
		return (data["durability"].at(0) = std::max(0, data["durability"].at(0).get<Durability>() - amount)) == 0;
	}

	bool ItemStack::hasAttribute(const Identifier &attribute) const {
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

	std::shared_ptr<Texture> ItemStack::getTexture(const Game &game) const {
		return item->getTexture(game);
	}

	void ItemStack::fromJSON(const Game &game, const nlohmann::json &json, ItemStack &stack) {
		Identifier id = json.at(0);
		stack.item = game.registries.get<ItemRegistry>().at(id);
		stack.count = json.at(1);
		if (2 < json.size()) {
			const auto &extra = json.at(2);
			if (extra.is_string() && extra == "with_durability") {
				const Durability durability = dynamic_cast<HasMaxDurability &>(*stack.item).maxDurability;
				stack.data["durability"] = std::make_pair(durability, durability);
			} else {
				stack.data = extra;
			}
		}
		stack.item->initStack(game, stack);
	}

	ItemStack ItemStack::fromJSON(const Game &game, const nlohmann::json &json) {
		ItemStack out(game);
		fromJSON(game, json, out);
		return out;
	}

	std::vector<ItemStack> ItemStack::manyFromJSON(const Game &game, const nlohmann::json &json) {
		std::vector<ItemStack> out;
		for (const auto &item: json)
			out.push_back(fromJSON(game, item));
		return out;
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->identifier;
		json[1] = stack.count;
		if (!stack.data.empty())
			json[2] = stack.data;
	}
}

std::ostream & operator<<(std::ostream &os, const Game3::ItemStack &stack) {
	return os << stack.item->name << " x " << stack.count;
}
