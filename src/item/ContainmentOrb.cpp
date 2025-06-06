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

namespace Game3 {
	bool ContainmentOrb::use(Slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		assert(realm->getSide() == Side::Server);
		GamePtr game = realm->getGame();
		PlayerPtr player = place.player;

		if (place.realm->type == "base:realm/shadow") {
			return true;
		}

		if (!stack->data.is_object()) {
			auto entities = realm->getEntities(place.position.getChunk());
			if (!entities) {
				WARN("No entities found in chunk {}", place.position.getChunk());
				return true;
			}

			EntityPtr selected;

			{
				auto lock = entities->sharedLock();
				for (const WeakEntityPtr &weak_entity: *entities) {
					if (EntityPtr entity = weak_entity.lock(); entity && entity->getPosition() == place.position && entity != player) {
						selected = entity;
						break;
					}
				}
			}

			if (!selected) {
				return true;
			}

			saveToJSON(selected, stack->data, true);
			player->getInventory(0)->notifyOwner(stack);

			return true;
		}

		if (!place.realm->isPathable(place.position)) {
			return true;
		}

		auto &object = stack->data.as_object();

		Identifier type = boost::json::value_to<Identifier>(object.at("type"));
		if (type == "base:entity/player") {
			game->toServer().releasePlayer(std::string(object.at("containedUsername").as_string()), place);
		} else {
			const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
			EntityPtr entity = (*factory)(game, stack->data);
			entity->spawning = true;
			entity->setRealm(realm);
			realm->queueEntityInit(std::move(entity), place.position);
		}
		object.clear();
		player->getInventory(0)->notifyOwner(stack);
		return true;
	}

	std::string ContainmentOrb::getTooltip(const ConstItemStackPtr &stack) {
		if (const auto *object = stack->data.if_object()) {
			if (auto *name = object->if_contains("containedName")) {
				return std::format("Containment Orb ({})", name->as_string());
			}
		}
		return "Containment Orb";
	}

	Identifier ContainmentOrb::getTextureIdentifier(const ConstItemStackPtr &stack) const {
		return stack->data.is_null()? "base:item/contorb" : "base:item/contorb_full";
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

	void ContainmentOrb::saveToJSON(const EntityPtr &entity, boost::json::value &json, bool can_modify) {
		auto &object = ensureObject(json);
		object["type"] = boost::json::value_from(entity->type);
		object["containedName"] = entity->getName();

		if (entity->isPlayer()) {
			auto player = safeDynamicCast<ServerPlayer>(entity);
			object["containedUsername"] = player->username;
			if (can_modify)
				player->teleport({32, 32}, entity->getGame()->getRealm(-1), MovementContext{.isTeleport = true});
		} else {
			entity->toJSON(json);
			if (can_modify) {
				entity->queueDestruction();
			}
		}
	}
}
