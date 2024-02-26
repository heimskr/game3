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
	bool ContainmentOrb::use(Slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		assert(realm->getSide() == Side::Server);
		GamePtr game = realm->getGame();
		PlayerPtr player = place.player;

		if (place.realm->type == "base:realm/shadow")
			return true;

		if (stack.data.empty()) {
			auto entities = realm->getEntities(place.position.getChunk());
			if (!entities) {
				WARN("No entities found in chunk {}", place.position.getChunk());
				return true;
			}

			EntityPtr selected;

			{
				auto lock = entities->sharedLock();
				for (const EntityPtr &entity: *entities) {
					if (entity->getPosition() == place.position && entity != player) {
						selected = entity;
						break;
					}
				}
			}

			if (!selected)
				return true;

			stack.data["containedEntity"] = selected->type;
			stack.data["containedName"] = selected->getName();

			if (selected->isPlayer()) {
				auto player = safeDynamicCast<ServerPlayer>(selected);
				player->teleport({32, 32}, game->getRealm(-1), MovementContext{.isTeleport = true});
				stack.data["containedUsername"] = player->username;
			} else {
				selected->toJSON(stack.data);
				selected->queueDestruction();
			}

			player->getInventory(0)->notifyOwner();
			SUCCESS("Captured {}", selected->type);
			return true;
		}

		if (!place.realm->isPathable(place.position))
			return true;

		Identifier type = stack.data.at("containedEntity");
		if (type == "base:entity/player") {
			game->toServer().releasePlayer(stack.data.at("containedUsername"), place);
		} else {
			const GlobalID new_gid = Agent::generateGID();
			const std::shared_ptr<EntityFactory> &factory = game->registry<EntityFactoryRegistry>()[type];
			EntityPtr entity = (*factory)(game, stack.data);
			entity->spawning = true;
			entity->setRealm(realm);
			realm->queueEntityInit(std::move(entity), place.position);
			INFO("Spawned entity of type {} with new GID {}", type, new_gid);
		}
		stack.data.clear();
		player->getInventory(0)->notifyOwner();
		return true;
	}

	std::string ContainmentOrb::getTooltip(const ItemStack &stack) {
		if (auto iter = stack.data.find("containedName"); iter != stack.data.end())
			return "Containment Orb (" + iter->get<std::string>() + ')';
		return "Containment Orb";
	}

	Identifier ContainmentOrb::getTextureIdentifier(const ItemStack &stack) const {
		return stack.data.empty()? "base:item/contorb" : "base:item/contorb_full";
	}
}
