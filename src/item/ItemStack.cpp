#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "item/HasMaxDurability.h"
#include "item/Item.h"
#include "item/ItemStack.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	ItemStack::ItemStack(const Game &game_):
		data(std::make_unique<nlohmann::json>()), game(&game_) {}

	ItemStack::ItemStack(const Game &game_, std::shared_ptr<Item> item_, ItemCount count_):
	item(std::move(item_)), count(count_), data(std::make_unique<nlohmann::json>()), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, std::shared_ptr<Item> item_, ItemCount count_, HeapObject<nlohmann::json> data_):
	item(std::move(item_)), count(count_), data(std::move(data_)), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, const ItemID &id, ItemCount count_):
	item(game_.registry<ItemRegistry>().at(id)), count(count_), data(nlohmann::json()), game(&game_) {
		assert(item);
		item->initStack(game_, *this);
	}

	ItemStack::ItemStack(const Game &game_, const ItemID &id, ItemCount count_, HeapObject<nlohmann::json> data_):
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
		if (!(item && !item->isTextureCacheable()) && cachedImage)
			return cachedImage;

		if (item)
			return cachedImage = item->getImage(game_, *this);

		return {};
	}

	ItemStack ItemStack::withCount(ItemCount new_count) const {
		assert(game);
		return {*game, item, new_count};
	}

	ItemStack::operator std::string() const {
		return item->getTooltip(*this) + " x " + std::to_string(count);
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id, Durability durability) {
		ItemStack out(game, id, 1);
		(*out.data)["durability"] = std::make_pair(durability, durability);
		return out;
	}

	ItemStack ItemStack::withDurability(const Game &game, const ItemID &id) {
		return withDurability(game, id, dynamic_cast<HasMaxDurability &>(*game.registry<ItemRegistry>()[id]).maxDurability);
	}

	bool ItemStack::reduceDurability(Durability amount) {
		if (!hasDurability())
			return false;
		return ((*data)["durability"].at(0) = std::max(0, (*data)["durability"].at(0).get<Durability>() - amount)) == 0;
	}

	bool ItemStack::hasAttribute(const Identifier &attribute) const {
		return item->attributes.contains(attribute);
	}

	bool ItemStack::hasDurability() const {
		if (auto iter = data->find("durability"); iter != data->end())
			return 0 <= iter->get<std::pair<double, double>>().second;
		return false;
	}

	double ItemStack::getDurabilityFraction() const {
		if (!hasDurability())
			return 1.;

		const std::pair<double, double> &pair = data->at("durability");
		return pair.first / pair.second;
	}

	void ItemStack::spawn(const std::shared_ptr<Realm> &realm, const Position &position) const {
		realm->spawn<ItemEntity>(position, *this);
	}

	std::shared_ptr<Texture> ItemStack::getTexture(const Game &) const {
		return item->getTexture(*this);
	}

	void ItemStack::fromJSON(const Game &game, const nlohmann::json &json, ItemStack &stack) {
		const Identifier id = json.at(0);
		stack.item = game.registry<ItemRegistry>()[id];
		stack.count = 1 < json.size()? json.at(1).get<ItemCount>() : 1;
		if (2 < json.size()) {
			const auto &extra = json.at(2);
			if (extra.is_string() && extra == "with_durability") {
				const Durability durability = dynamic_cast<HasMaxDurability &>(*stack.item).maxDurability;
				(*stack.data)["durability"] = std::make_pair(durability, durability);
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

	void ItemStack::encode(Game &game_, Buffer &buffer) {
		absorbGame(game_);
		buffer << item->identifier;
		buffer << count;
		buffer << data->dump();
	}

	void ItemStack::decode(Game &game_, Buffer &buffer) {
		absorbGame(game_);
		item = game->registry<ItemRegistry>()[buffer.take<Identifier>()];
		buffer >> count;
		data = nlohmann::json::parse(buffer.take<std::string>());
	}

	void ItemStack::absorbGame(const Game &game_) {
		if (game == nullptr)
			game = &game_;
		else
			assert(game == &game_);
	}

	template <>
	std::string Buffer::getType(const ItemStack &) {
		return {'\xe0'};
	}

	template <>
	ItemStack popBuffer<ItemStack>(Buffer &buffer) {
		auto context = buffer.context.lock();
		assert(context);
		const auto &game = dynamic_cast<Game &>(*context);
		ItemStack out(game);
		buffer >> out;
		return out;
	}

	Buffer & operator+=(Buffer &buffer, const ItemStack &stack) {
		assert(stack.item);
		buffer.appendType(stack);
		buffer += stack.item->identifier;
		buffer += stack.count;
		buffer += stack.data->dump(); // TODO: Buffer::operator+= for json
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const ItemStack &stack) {
		return buffer += stack;
	}

	Buffer & operator>>(Buffer &buffer, ItemStack &stack) {
		assert(stack.hasGame());
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(stack))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected Inventory)");
		}
		const auto item_id = popBuffer<Identifier>(buffer);
		popBuffer(buffer, stack.count);
		const auto raw_json = popBuffer<std::string>(buffer); // TODO: popBuffer for json
		stack.data = nlohmann::json::parse(raw_json);
		stack.item = stack.getGame().registry<ItemRegistry>().at(item_id);
		return buffer;
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->identifier;
		json[1] = stack.count;
		if (!stack.data->empty())
			json[2] = stack.data;
	}

	std::ostream & operator<<(std::ostream &os, const Game3::ItemStack &stack) {
		return os << stack.item->getTooltip(stack) << " x " << stack.count;
	}
}
