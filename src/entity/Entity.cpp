#include <iostream>
#include <sstream>

#include "Tiles.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "util/Util.h"

namespace Game3 {
	std::unordered_map<EntityID, EntityTexture> Entity::textureMap {
		{Entity::GANGBLANC, {Texture("resources/characters/champions/Gangblanc.png"), 1}},
		{Entity::GRUM,      {Texture("resources/characters/champions/Grum.png"),      1}},
		{Entity::ITEM,      {Texture("resources/missing.png"),                        0}}, // Rendering is handled on a per-item basis by the ItemEntity class
		{Entity::VILLAGER1, {Texture("resources/characters/villager1.png"),           2}},
	};

	std::shared_ptr<Entity> Entity::fromJSON(const nlohmann::json &json) {
		const EntityID id = json.at("id");
		std::shared_ptr<Entity> out;

		if (json.contains("isPlayer") && json.at("isPlayer") == true)
			out = Entity::create<Player>(id);
		else
			switch (id) {
				case Entity::ITEM:
					out = ItemEntity::create(json.at("stack"));
					break;
				default:
					out = Entity::create<Entity>(id);
					break;
			}

		out->absorbJSON(json);
		out->init();
		return out;
	}

	nlohmann::json Entity::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Entity::absorbJSON(const nlohmann::json &json) {
		id(json.at("id"));
		position = json.at("position");
		realmID = json.at("realmID");
		direction = json.at("direction");
		if (json.contains("inventory"))
			inventory = std::make_shared<Inventory>(Inventory::fromJSON(json.at("inventory"), shared_from_this()));
	}

	void Entity::tick(float delta) {
		auto &x = offset.x();
		auto &y = offset.y();
		if (x < 0.f)
			x = std::min(x + delta * speed, 0.f);
		else if (0.f < x)
			x = std::max(x - delta * speed, 0.f);
		if (y < 0.f)
			y = std::min(y + delta * speed, 0.f);
		else if (0.f < y)
			y = std::max(y - delta * speed, 0.f);
	}

	void Entity::remove() {
		auto realm = getRealm();
		// I'm assuming this has to be in its own variable to prevent the destructor from being called before this function returns.
		auto shared = shared_from_this();
		realm->entities.erase(shared);
	}

	void Entity::id(EntityID new_id) {
		id_ = new_id;
		if (texture == nullptr)
			texture = &textureMap.at(id_).texture;
	}

	void Entity::init() {
		if (texture == nullptr)
			texture = &textureMap.at(id_).texture;

		if (!inventory)
			inventory = std::make_shared<Inventory>(shared_from_this(), DEFAULT_INVENTORY_SIZE);
		else
			inventory->owner = shared_from_this();
	}

	void Entity::render(SpriteRenderer &sprite_renderer) const {
		if (texture == nullptr)
			return;

		float x_offset = 0.f;
		float y_offset = 0.f;
		if (offset.x() != 0.f || offset.y() != 0.f)
			x_offset = 8.f * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - getRealm()->getGame().startTime).count() / 100) % 5);

		switch (variety) {
			case 1:
				y_offset = 8.f * int(direction);
				break;
			case 2:
				y_offset = 16.f * int(remapDirection(direction, 0x0213));
				break;
		}

		sprite_renderer.drawOnMap(*texture, position.column + offset.x(), position.row + offset.y(), x_offset, y_offset, 16.f, 16.f);
	}

	void Entity::move(Direction move_direction) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		Position new_position = position;
		float x_offset = 0.f;
		float y_offset = 0.f;
		bool horizontal = false;
		switch (move_direction) {
			case Direction::Down:
				++new_position.row;
				direction = Direction::Down;
				y_offset = -1.f;
				break;
			case Direction::Up:
				--new_position.row;
				direction = Direction::Up;
				y_offset = 1.f;
				break;
			case Direction::Left:
				--new_position.column;
				direction = Direction::Left;
				x_offset = 1.f;
				horizontal = true;
				break;
			case Direction::Right:
				++new_position.column;
				direction = Direction::Right;
				x_offset = -1.f;
				horizontal = true;
				break;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(int(move_direction)));
		}

		if ((horizontal && offset.x() != 0) || (!horizontal && offset.y() != 0))
			return;

		if (canMoveTo(new_position)) {
			teleport(new_position, false);
			if (horizontal)
				offset.x() = x_offset;
			else
				offset.y() = y_offset;
		}
	}

	std::shared_ptr<Realm> Entity::getRealm() const {
		auto out = weakRealm.lock();
		if (!out)
			throw std::runtime_error("Couldn't lock entity's realm");
		return out;
	}

	void Entity::setRealm(const Game &game, RealmID realm_id) {
		weakRealm = game.realms.at(realm_id);
		realmID = realm_id;
	}

	void Entity::setRealm(const std::shared_ptr<Realm> realm) {
		weakRealm = realm;
		realmID = realm->id;
	}

	Entity::Entity(EntityID id__): id_(id__) {
		variety = textureMap.at(id_).variety;
	}

	bool Entity::canMoveTo(const Position &new_position) const {
		if (new_position.row < 0 || new_position.column < 0)
			return false;

		auto realm = weakRealm.lock();
		if (!realm)
			return false;

		if (realm->getHeight() <= new_position.row || realm->getWidth() <= new_position.column)
			return false;

		const auto &tileset = *tileSets.at(realm->type);

		if (!tileset.isWalkable((*realm->tilemap1)(new_position.column, new_position.row)))
			return false;

		if (tileset.isSolid((*realm->tilemap2)(new_position.column, new_position.row)))
			return false;

		if (auto tile_entity = realm->tileEntityAt(new_position))
			if (tile_entity->solid)
				return false;

		return true;
	}

	void Entity::focus(Canvas &canvas, bool is_autofocus) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		if (!is_autofocus)
			canvas.scale = 4.f;
		else if (++canvas.autofocusCounter < Canvas::AUTOFOCUS_DELAY)
			return;

		canvas.autofocusCounter = 0;
		const auto &tilemap = *realm->tilemap1;
		canvas.center.x() = -(column() - tilemap.width  / 2.f + 0.5f) - offset.x();
		canvas.center.y() = -(row()    - tilemap.height / 2.f + 0.5f) - offset.y();
	}

	void Entity::teleport(const Position &new_position, bool clear_offset) {
		position = new_position;
		if (clear_offset)
			offset = {0.f, 0.f};
		auto shared = shared_from_this();
		getRealm()->onMoved(shared, new_position);
		for (auto iter = moveQueue.begin(); iter != moveQueue.end();) {
			if ((*iter)(shared))
				moveQueue.erase(iter++);
			else
				++iter;
		}
	}

	void Entity::teleport(const Position &new_position, const std::shared_ptr<Realm> &new_realm) {
		auto old_realm = getRealm();
		auto shared = shared_from_this();
		old_realm->queueRemoval(shared);
		new_realm->add(shared);
		teleport(new_position);
	}

	void Entity::teleport(Index index, const std::shared_ptr<Realm> &new_realm) {
		teleport(new_realm->getPosition(index), new_realm);
	}

	Position Entity::nextTo() const {
		switch (direction) {
			case Direction::Up:    return {position.row - 1, position.column};
			case Direction::Down:  return {position.row + 1, position.column};
			case Direction::Left:  return {position.row, position.column - 1};
			case Direction::Right: return {position.row, position.column + 1};
			default: throw std::invalid_argument("Invalid direction: " + std::to_string(int(direction)));
		}
	}

	std::string Entity::debug() const {
		std::stringstream sstream;
		sstream << "Entity[type=" << id_ << ", position=" << position << ", realm=" << realmID << ", direction=" << direction << ']';
		return sstream.str();
	}

	void Entity::queueForMove(const std::function<bool(const std::shared_ptr<Entity> &)> &function) {
		moveQueue.push_back(function);
	}

	void to_json(nlohmann::json &json, const Entity &entity) {
		json["id"] = entity.id();
		json["position"] = entity.position;
		json["realmID"] = entity.realmID;
		json["direction"] = entity.direction;
		if (entity.inventory)
			json["inventory"] = *entity.inventory;
	}
}
