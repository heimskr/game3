#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "item/ContainmentOrb.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "types/Position.h"
#include "util/Cast.h"
#include "util/JSON.h"

#include "FastNoise2/src/FastNoise/Base64.h"

namespace Game3 {
	bool ContainmentOrb::use(Slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		assert(place.realm->getSide() == Side::Server);

		if (place.realm->type == "base:realm/shadow") {
			return true;
		}

		auto &data = stack->data;
		auto data_lock = data.uniqueLock();
		boost::json::object &object = ensureObject(data);

		bool &dense = boolifyKey(object, "dense", false);

		// Shift-ctrl click to toggle density
		if (modifiers == Modifiers(true, true, false, false)) {
			dense = !dense;
			place.player->getInventory(0)->notifyOwner(stack);
		} else if (dense? denseClick(place, object, modifiers.onlyShift()) : regularClick(place, object)) {
			place.player->getInventory(0)->notifyOwner(stack);
		}

		return true;
	}

	bool ContainmentOrb::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction action) {
		if (action == DragAction::Update) {
			return use(slot, stack, place, modifiers, offsets);
		}

		return false;
	}

	std::string ContainmentOrb::getTooltip(const ConstItemStackPtr &stack) {
		auto lock = stack->data.sharedLock();

		if (const boost::json::object *object = stack->data.if_object()) {
			if (getBoolKey(*object, "dense", false)) {
				if (const boost::json::value *entities_value = object->if_contains("entities")) {
					if (const boost::json::array *entities = entities_value->if_array()) {
						if (!entities->empty()) {
							return std::format("Dense Containment Orb ({})", entities->size());
						}
					}
				}

				return "Dense Containment Orb";
			}

			if (const boost::json::value *entity_value = object->if_contains("entity")) {
				if (const boost::json::object *entity = entity_value->if_object()) {
					if (const boost::json::value *name = entity->if_contains("containedName")) {
						return std::format("Containment Orb ({})", name->as_string());
					}
				}
			}
		}

		return "Containment Orb";
	}

	Identifier ContainmentOrb::getTextureIdentifier(const ConstItemStackPtr &stack) const {
		auto lock = stack->data.sharedLock();
		bool empty = true;

		if (const boost::json::object *object = stack->data.if_object()) {
			if (getBoolKey(*object, "dense", false)) {
				if (const boost::json::value *entities_value = object->if_contains("entities")) {
					if (const boost::json::array *entities = entities_value->if_array()) {
						empty = entities->empty();
					}
				}
			} else if (const boost::json::value *entity_value = object->if_contains("entity")) {
				empty = entity_value->is_object();
			}
		}

		return empty? "base:item/contorb" : "base:item/contorb_full";
	}

	EntityPtr ContainmentOrb::makeEntity(const ItemStackPtr &stack) {
		GamePtr game = stack->getGame();
		Identifier type = boost::json::value_to<Identifier>(stack->data.at("type"));
		const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
		EntityPtr entity = (*factory)(game, stack->data);
		entity->spawning = true;
		return entity;
	}

	bool ContainmentOrb::validate(const ItemStackPtr &stack) {
		return stack && stack->getID() == "base:item/contorb";
	}

	bool ContainmentOrb::isEmpty(const ItemStackPtr &stack) {
		if (!stack) {
			throw std::invalid_argument("Can't evaluate whether null stack is an empty containment orb");
		}

		if (stack->getID() != "base:item/contorb") {
			throw std::invalid_argument("Can't evaluate whether non-containment orb stack is an empty containment orb");
		}

		if (const auto *object = stack->data.if_object()) {
			return object->empty();
		}

		return true;
	}

	void ContainmentOrb::saveToJSON(const EntityPtr &entity, boost::json::value &value, bool can_modify) {
		boost::json::object &object = ensureObject(value);

		if (entity->isPlayer()) {
			auto player = safeDynamicCast<ServerPlayer>(entity);
			object["containedUsername"] = player->username;
			if (can_modify) {
				player->teleport({32, 32}, entity->getGame()->getRealm(-1), MovementContext{.isTeleport = true});
			}
		} else {
			entity->toJSON(value);
			if (can_modify) {
				entity->queueDestruction();
			}
		}

		Buffer buffer(entity->getGame(), Side::Server);
		entity->encode(buffer);
		object["type"] = boost::json::value_from(entity->type);
		object["containedName"] = entity->getName();
		object["buffer"] = FastNoise::Base64::Encode(buffer.bytes);
	}

	bool ContainmentOrb::denseClick(const Place &place, boost::json::object &object, bool release) {
		boost::json::array &entities = ensureArray(object["entities"]);

		if (release) {
			if (entities.empty()) {
				return false;
			}

			ensureObject(entities.back());

			if (releaseEntity(place, entities.back())) {
				entities.pop_back();
				return true;
			}

			return false;
		}

		if (EntityPtr taken = takeEntity(place)) {
			boost::json::value value;
			saveToJSON(taken, value, true);
			entities.push_back(std::move(value));
			return true;
		}

		return false;
	}

	bool ContainmentOrb::regularClick(const Place &place, boost::json::object &object) {
		RealmPtr realm = place.realm;

		if (boost::json::value *entity_value = object.if_contains("entity")) {
			if (releaseEntity(place, *entity_value)) {
				object.erase("entity");
				return true;
			}
			return false;
		}

		if (EntityPtr taken = takeEntity(place)) {
			saveToJSON(taken, object["entity"], true);
			return true;
		}

		return false;
	}

	EntityPtr ContainmentOrb::takeEntity(const Place &place) {
		if (auto entities = place.realm->getEntities(place.position.getChunk())) {
			auto lock = entities->sharedLock();
			for (const WeakEntityPtr &weak_entity: *entities) {
				if (EntityPtr entity = weak_entity.lock(); entity && entity->getPosition() == place.position && entity != place.player) {
					return entity;
				}
			}
		} else {
			WARN("No entities found in chunk {}", place.position.getChunk());
		}

		return nullptr;
	}

	bool ContainmentOrb::releaseEntity(const Place &place, boost::json::value &value) {
		if (!place.realm->isPathable(place.position)) {
			return false;
		}

		boost::json::object &object = ensureObject(value);

		Identifier type = boost::json::value_to<Identifier>(object.at("type"));
		GamePtr game = place.getGame();
		if (type == "base:entity/player") {
			game->toServer().releasePlayer(std::string(object.at("containedUsername").as_string()), place);
		} else {
			const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
			EntityPtr entity = (*factory)(game, value);
			entity->spawning = true;
			entity->setRealm(place.realm);
			const boost::json::string &base64 = object.at("buffer").as_string();
			std::vector<uint8_t> bytes = FastNoise::Base64::Decode(base64.c_str());
			Buffer buffer(std::move(bytes), game, Side::Server);
			entity->decode(buffer);
			place.realm->queueAddition(entity, place.position);
		}

		return true;
	}
}
