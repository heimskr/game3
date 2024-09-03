#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/ContainmentOrb.h"
#include "realm/Realm.h"
#include "util/Cast.h"

namespace Game3 {
	bool ContainmentOrb::use(Slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		assert(realm->getSide() == Side::Server);
		GamePtr game = realm->getGame();
		PlayerPtr player = place.player;

		if (place.realm->type == "base:realm/shadow")
			return true;

		if (stack->data.empty()) {
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

			if (!selected)
				return true;

			saveToJSON(selected, stack->data, true);
			player->getInventory(0)->notifyOwner();

			return true;
		}

		if (!place.realm->isPathable(place.position))
			return true;

		Identifier type = stack->data.at("type");
		if (type == "base:entity/player") {
			game->toServer().releasePlayer(stack->data.at("containedUsername"), place);
		} else {
			const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
			EntityPtr entity = (*factory)(game, stack->data);
			entity->spawning = true;
			entity->setRealm(realm);
			realm->queueEntityInit(std::move(entity), place.position);
		}
		stack->data.clear();
		player->getInventory(0)->notifyOwner();
		return true;
	}

	std::string ContainmentOrb::getTooltip(const ConstItemStackPtr &stack) {
		if (auto iter = stack->data.find("containedName"); iter != stack->data.end())
			return "Containment Orb (" + iter->get<std::string>() + ')';
		return "Containment Orb";
	}

	Identifier ContainmentOrb::getTextureIdentifier(const ConstItemStackPtr &stack) const {
		return stack->data.empty()? "base:item/contorb" : "base:item/contorb_full";
	}

	EntityPtr ContainmentOrb::makeEntity(const ItemStackPtr &stack) {
		GamePtr game = stack->getGame();
		Identifier type = stack->data.at("type");
		const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
		EntityPtr entity = (*factory)(game, stack->data);
		entity->spawning = true;
		return entity;
	}

	bool ContainmentOrb::validate(const ItemStackPtr &stack) {
		return stack && stack->getID() == "base:item/contorb";
	}

	bool ContainmentOrb::isEmpty(const ItemStackPtr &stack) {
		if (!stack)
			throw std::invalid_argument("Can't evaluate whether null stack is an empty containment orb");

		if (stack->getID() != "base:item/contorb")
			throw std::invalid_argument("Can't evaluate whether non-containment orb stack is an empty containment orb");

		return stack->data.empty();
	}

	void ContainmentOrb::saveToJSON(const EntityPtr &entity, nlohmann::json &json, bool can_modify) {
		json["type"] = entity->type;
		json["containedName"] = entity->getName();

		if (entity->isPlayer()) {
			auto player = safeDynamicCast<ServerPlayer>(entity);
			json["containedUsername"] = player->username;
			if (can_modify)
				player->teleport({32, 32}, entity->getGame()->getRealm(-1), MovementContext{.isTeleport = true});
		} else {
			entity->toJSON(json);
			if (can_modify)
				entity->queueDestruction();
		}
	}
}
