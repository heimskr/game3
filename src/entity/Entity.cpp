#include <nanogui/opengl.h>

#include <iostream>
#include <sstream>

#include "Tiles.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	std::unordered_map<EntityID, Texture> Entity::textureMap {
		{Entity::GANGBLANC, Texture("resources/characters/champions/Gangblanc.png")},
		{Entity::GRUM,      Texture("resources/characters/champions/Grum.png")},
		{Entity::ITEM,      Texture("resources/missing.png")}, // Rendering is handled on a per-item basis by the ItemEntity class
	};

	std::shared_ptr<Entity> Entity::fromJSON(const nlohmann::json &json) {
		std::shared_ptr<Entity> out;
		const EntityID id = json.at("id");

		if (json.at("isPlayer") == true)
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
		inventory = Inventory::fromJSON(json.at("inventory"), shared_from_this());
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

	void Entity::id(EntityID new_id) {
		id_ = new_id;
		texture = &textureMap.at(id_);
	}

	void Entity::init() {
		texture = &textureMap.at(id_);
		inventory.owner = shared_from_this();
	}

	void Entity::render(SpriteRenderer &sprite_renderer) const {
		if (!texture)
			return;

		sprite_renderer.drawOnMap(*texture, position.column + offset.x(), position.row + offset.y(), 0.f, 8.f * int(direction), 16.f, 16.f);
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
			position = new_position;
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

	bool Entity::canMoveTo(const Position &new_position) const {
		if (new_position.row < 0 || new_position.column < 0)
			return false;

		auto realm = weakRealm.lock();
		if (!realm)
			return false;

		if (realm->getHeight() <= new_position.row || realm->getWidth() <= new_position.column)
			return false;

		if (!isLand((*realm->tilemap1)(new_position.column, new_position.row)))
			return false;

		if (isSolid((*realm->tilemap2)(new_position.column, new_position.row)))
			return false;

		return true;
	}

	void Entity::focus(Canvas &canvas) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		const auto &tilemap = *realm->tilemap1;
		canvas.scale = 4.f;
		canvas.center.x() = -(column() - tilemap.width  / 2.f);
		canvas.center.y() = -(row()    - tilemap.height / 2.f);
	}

	void Entity::teleport(const Position &new_position) {
		position = new_position;
		offset = {0.f, 0.f};
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

	void to_json(nlohmann::json &json, const Entity &entity) {
		json["id"] = entity.id();
		json["position"] = entity.position;
		json["realmID"] = entity.realmID;
		json["direction"] = entity.direction;
		json["inventory"] = entity.inventory;
	}
}
