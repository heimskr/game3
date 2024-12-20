#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "graphics/ItemTexture.h"
#include "item/HasMaxDurability.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {

// ItemTexture

	ItemTexture::ItemTexture(Identifier identifier_, const std::shared_ptr<Texture> &texture_, int x_, int y_, int width_, int height_):
		NamedRegisterable(std::move(identifier_)),
		texture(texture_),
		x(x_),
		y(y_),
		width(width_),
		height(height_) {}

	std::shared_ptr<Texture> ItemTexture::getTexture() {
		if (auto locked = texture.lock())
			return locked;
		throw std::runtime_error("Couldn't lock ItemTexture's texture");
	}

	ItemTexture::operator bool() const {
		return x != -1 && y != -1 && texture.lock() && width != -1 && height != -1;
	}

// Item

	Item::Item(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count):
		NamedRegisterable(std::move(id_)), name(std::move(name_)), basePrice(base_price), maxCount(max_count) {}

	Item::~Item() = default;

	TexturePtr Item::getTexture(const Game &game, const ConstItemStackPtr &stack) const {
		if (!isTextureCacheable() || !cachedTexture)
			cachedTexture = makeTexture(game, stack);
		return cachedTexture;
	}

	TexturePtr Item::makeTexture(const Game &game, const ConstItemStackPtr &stack) const {
		auto item_texture = game.registry<ItemTextureRegistry>().at(getTextureIdentifier(stack));
		auto texture = item_texture->getTexture();
		texture->init();
		const int width  = item_texture->width;
		const int height = item_texture->height;
		const ptrdiff_t channels = texture->format == GL_RGBA? 4 : 3;
		const size_t row_size = channels * width;

		auto lock = rawImage.uniqueLock();

		if (!rawImage || !isTextureCacheable()) {
			rawImage = std::make_shared<uint8_t[]>(channels * width * height);
			uint8_t *raw_pointer = rawImage.get();
			uint8_t *texture_pointer = texture->data.get() + item_texture->y * texture->width * channels + static_cast<ptrdiff_t>(item_texture->x) * channels;
			for (int row = 0; row < height; ++row) {
				std::memcpy(raw_pointer + row_size * row, texture_pointer, row_size);
				texture_pointer += static_cast<ptrdiff_t>(channels) * texture->width;
			}
		}

		TexturePtr new_texture = std::make_shared<Texture>(texture->identifier);
		new_texture->alpha = texture->alpha;
		new_texture->filter = texture->filter;
		new_texture->format = texture->format;
		new_texture->init(rawImage, width, height);

		return new_texture;
	}

	Identifier Item::getTextureIdentifier(const ConstItemStackPtr &) const {
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

	Item & Item::addAttribute(Identifier attribute) & {
		attributes.insert(std::move(attribute));
		return *this;
	}

	Item && Item::addAttribute(Identifier attribute) && {
		attributes.insert(std::move(attribute));
		return std::move(*this);
	}

	bool Item::hasAttribute(const Identifier &attribute) const {
		return attributes.contains(attribute);
	}

	std::shared_ptr<ItemTexture> Item::getItemTexture(const ConstItemStackPtr &stack) {
		if (isTextureCacheable() && cachedItemTexture) {
			return cachedItemTexture;
		}

		GamePtr game = stack->getGame();
		return cachedItemTexture = game->registry<ItemTextureRegistry>().at(identifier);
	}

	std::string Item::getTooltip(const ConstItemStackPtr &) {
		return name;
	}

	bool Item::use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) {
		return false;
	}

	bool Item::use(Slot, const ItemStackPtr &, const Place &, Modifiers, Hand) {
		return false;
	}

	bool Item::use(Slot, const ItemStackPtr &, const PlayerPtr &, Modifiers) {
		return false;
	}

	bool Item::drag(Slot, const ItemStackPtr &, const Place &, Modifiers) {
		return false;
	}

// ItemStack

	ItemStack::ItemStack(const GamePtr &game):
		weakGame(game) {}

	ItemStack::ItemStack(const GamePtr &game, std::shared_ptr<Item> item_, ItemCount count_):
	item(std::move(item_)), count(count_), weakGame(game) {
		assert(item);
		item->initStack(*game, *this);
	}

	ItemStack::ItemStack(const GamePtr &game, std::shared_ptr<Item> item_, ItemCount count_, nlohmann::json data_):
	item(std::move(item_)), count(count_), data(std::move(data_)), weakGame(game) {
		assert(item);
		item->initStack(*game, *this);
	}

	ItemStack::ItemStack(const GamePtr &game, const ItemID &id, ItemCount count_):
	item(game->registry<ItemRegistry>().at(id)), count(count_), weakGame(game) {
		assert(item);
		item->initStack(*game, *this);
	}

	ItemStack::ItemStack(const GamePtr &game, const ItemID &id, ItemCount count, nlohmann::json data):
		item(game->registry<ItemRegistry>().at(id)), count(count), data(std::move(data)), weakGame(game) {
			assert(item != nullptr);
			item->initStack(*game, *this);
		}

	TexturePtr ItemStack::getTexture() const {
		GamePtr game = getGame();
		return getTexture(*game);
	}

	TexturePtr ItemStack::getTexture(const Game &game) const {
		if (!(item && !item->isTextureCacheable()) && cachedTexture) {
			return cachedTexture;
		}

		if (item) {
			return cachedTexture = item->getTexture(game, shared_from_this());
		}

		return {};
	}

	bool ItemStack::canMerge(const ItemStack &other) const {
		if (!item || !other.item) {
			return false;
		}

		return *item == *other.item && data == other.data;
	}

	ItemStackPtr ItemStack::withCount(ItemCount new_count) const {
		return ItemStack::create(getGame(), item, new_count, data);
	}

	ItemStackPtr ItemStack::withDurability(const GamePtr &game, const ItemID &id, Durability durability) {
		ItemStackPtr out = ItemStack::create(game, id, 1);
		out->data["durability"] = std::make_pair(durability, durability);
		return out;
	}

	ItemStackPtr ItemStack::withDurability(const GamePtr &game, const ItemID &id) {
		return withDurability(game, id, dynamic_cast<HasMaxDurability &>(*game->registry<ItemRegistry>()[id]).maxDurability);
	}

	bool ItemStack::reduceDurability(Durability amount) {
		if (!hasDurability()) {
			return false;
		}

		return (data["durability"].at(0) = std::max(0, data["durability"].at(0).get<Durability>() - amount)) == 0;
	}

	bool ItemStack::hasAttribute(const Identifier &attribute) const {
		return item->attributes.contains(attribute);
	}

	bool ItemStack::hasDurability() const {
		if (auto iter = data.find("durability"); iter != data.end()) {
			return 0 <= iter->get<std::pair<double, double>>().second;
		}

		return false;
	}

	double ItemStack::getDurabilityFraction() const {
		if (!hasDurability()) {
			return 1.;
		}

		const std::pair<double, double> &pair = data.at("durability");
		return pair.first / pair.second;
	}

	std::string ItemStack::getTooltip() const {
		assert(item);
		return item->getTooltip(shared_from_this());
	}

	void ItemStack::spawn(const Place &place) const {
		assert(place.realm);
		place.realm->spawn<ItemEntity>(place.position, copy());
	}

	std::shared_ptr<ItemTexture> ItemStack::getItemTexture(Game &) const {
		return item->getItemTexture(shared_from_this());
	}

	void ItemStack::fromJSON(const GamePtr &game, const nlohmann::json &json, ItemStack &stack) {
		const Identifier id = json.at(0);
		stack.item = game->registry<ItemRegistry>()[id];
		stack.count = 1 < json.size()? json.at(1).get<ItemCount>() : 1;
		if (2 < json.size()) {
			const auto &extra = json.at(2);
			if (extra.is_string() && extra == "with_durability") {
				const Durability durability = dynamic_cast<HasMaxDurability &>(*stack.item).maxDurability;
				stack.data["durability"] = std::make_pair(durability, durability);
			} else {
				stack.data = extra;
			}
		}
		stack.item->initStack(*game, stack);
	}

	ItemStackPtr ItemStack::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		auto out = ItemStack::create(game);
		fromJSON(game, json, *out);
		return out;
	}

	std::vector<ItemStackPtr> ItemStack::manyFromJSON(const GamePtr &game, const nlohmann::json &json) {
		std::vector<ItemStackPtr> out;
		for (const auto &item: json) {
			out.push_back(fromJSON(game, item));
		}
		return out;
	}

	void ItemStack::onDestroy() {
		GamePtr game = getGame();
		item->onDestroy(*game, shared_from_this());
	}

	void ItemStack::onDestroy(Game &game) {
		item->onDestroy(game, shared_from_this());
	}

	void ItemStack::renderEffects(const RendererContext &context, const Position &position, Modifiers modifiers) {
		item->renderEffects(context, position, modifiers, shared_from_this());
	}

	void ItemStack::encode(Game &game, Buffer &buffer) {
		absorbGame(game);
		buffer << item->identifier;
		buffer << count;
		buffer << data.dump();
	}

	void ItemStack::decode(Game &game, Buffer &buffer) {
		absorbGame(game);
		item = game.registry<ItemRegistry>()[buffer.take<Identifier>()];
		buffer >> count;
		data = nlohmann::json::parse(buffer.take<std::string>());
	}

	ItemStackPtr ItemStack::copy() const {
		return ItemStack::create(getGame(), item, count, data);
	}

	void ItemStack::absorbGame(Game &game) {
		if (weakGame.expired()) {
			weakGame = game.weak_from_this();
		} else {
			// TODO: Game::operator==
			assert(weakGame.lock().get() == &game);
		}
	}

	template <>
	std::string Buffer::getType(const ItemStack &, bool) {
		return {'\xe0'};
	}

	template <>
	std::string Buffer::getType(const ItemStackPtr &, bool) {
		return {'\xe0'};
	}

	template <>
	ItemStackPtr popBuffer<ItemStackPtr>(Buffer &buffer) {
		auto context = buffer.context.lock();
		assert(context);
		auto &game = dynamic_cast<Game &>(*context);
		ItemStackPtr out = ItemStack::create(game.shared_from_this());
		buffer >> out;
		return out;
	}

	Buffer & operator+=(Buffer &buffer, const ItemStackPtr &stack) {
		assert(stack->item);
		buffer.appendType(stack, false);
		buffer << stack->item->identifier;
		buffer << stack->count;
		buffer << stack->data;
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const ItemStackPtr &stack) {
		return buffer += stack;
	}

	Buffer & operator>>(Buffer &buffer, ItemStackPtr &stack) {
		if (!stack) {
			stack = ItemStack::create(safeDynamicCast<Game>(buffer.context.lock()));
		}

		const std::string type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(stack, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected ItemStack)");
		}
		const Identifier item_id = buffer.take<Identifier>();
		stack->count = buffer.take<ItemCount>();
		stack->data = buffer.take<nlohmann::json>();
		stack->item = stack->getGame()->registry<ItemRegistry>().at(item_id);
		return buffer;
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->identifier;
		json[1] = stack.count;
		if (!stack.data.empty()) {
			json[2] = stack.data;
		}
	}

	template <>
	ItemStackPtr makeForBuffer<ItemStackPtr>(Buffer &buffer) {
		auto context = buffer.context.lock();
		assert(context);
		auto game = std::dynamic_pointer_cast<Game>(context);
		assert(game);
		return ItemStack::create(game);
	}
}
