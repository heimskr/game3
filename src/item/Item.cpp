#include <iostream>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "item/Item.h"
#include "item/ItemStack.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "util/Util.h"

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

	Glib::RefPtr<Gdk::Pixbuf> Item::getImage(const Game &game, const ItemStack &stack) {
		if (!isTextureCacheable() || !cachedImage)
			cachedImage = makeImage(game, stack);
		return cachedImage;
	}

	Glib::RefPtr<Gdk::Pixbuf> Item::makeImage(const Game &game, const ItemStack &stack) {
		auto item_texture = game.registry<ItemTextureRegistry>().at(getTextureIdentifier(stack));
		auto texture = item_texture->getTexture(game);
		texture->init();
		const int width  = item_texture->width;
		const int height = item_texture->height;
		const ptrdiff_t channels = texture->format == GL_RGBA? 4 : 3;
		const size_t row_size = channels * width;

		if (!rawImage || !isTextureCacheable()) {
			rawImage = std::make_unique<uint8_t[]>(channels * width * height);
			uint8_t *raw_pointer = rawImage.get();
			uint8_t *texture_pointer = texture->data.get() + item_texture->y * texture->width * channels + static_cast<ptrdiff_t>(item_texture->x) * channels;
			for (int row = 0; row < height; ++row) {
				std::memcpy(raw_pointer + row_size * row, texture_pointer, row_size);
				texture_pointer += static_cast<ptrdiff_t>(channels) * texture->width;
			}
		}

		constexpr int doublings = 3;
		return Gdk::Pixbuf::create_from_data(rawImage.get(), Gdk::Colorspace::RGB, texture->alpha, 8, width, height, int(row_size))
		       ->scale_simple(width << doublings, height << doublings, Gdk::InterpType::NEAREST);
	}

	Identifier Item::getTextureIdentifier(const ItemStack &) {
		return identifier;
	}

	void Item::getOffsets(const Game &game, std::shared_ptr<Texture> &texture, float &x_offset, float &y_offset) {
		const auto &registry = game.registry<ItemTextureRegistry>();
		if (registry.contains(identifier)) {
			auto item_texture = registry.at(identifier);
			texture  = item_texture->texture.lock();
			x_offset = static_cast<float>(item_texture->x) / 2.f;
			y_offset = static_cast<float>(item_texture->y) / 2.f;
		} else {
			x_offset = 0.f;
			y_offset = 0.f;
		}
	}

	std::shared_ptr<Item> Item::addAttribute(Identifier attribute) {
		attributes.insert(std::move(attribute));
		return shared_from_this();
	}

	std::shared_ptr<Texture> Item::getTexture(const ItemStack &stack) {
		if (isTextureCacheable() && cachedTexture)
			return cachedTexture;

		const Game &game = stack.getGame();
		return cachedTexture = game.registry<ItemTextureRegistry>().at(identifier)->getTexture(game);
	}

	std::string Item::getTooltip(const ItemStack &) {
		return name;
	}

	bool Item::use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) {
		return false;
	}

	bool Item::drag(Slot, ItemStack &, const Place &, Modifiers) {
		return false;
	}
}
