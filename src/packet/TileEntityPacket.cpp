#include "Log.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "net/Buffer.h"
#include "packet/PacketError.h"
#include "packet/TileEntityPacket.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"

namespace Game3 {
	TileEntityPacket::TileEntityPacket(TileEntityPtr tile_entity):
		tileEntity(std::move(tile_entity)),
		identifier(tileEntity->getID()),
		globalID(tileEntity->getGID()),
		realmID(tileEntity->realmID) {}

	void TileEntityPacket::decode(Game &, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		assert(globalID != GlobalID(-1));
		assert(globalID != GlobalID(0));
		storedBuffer = std::move(buffer);
	}

	void TileEntityPacket::encode(Game &game, Buffer &buffer) const {
		assert(tileEntity);
		buffer << globalID << identifier << realmID;
		tileEntity->encode(game, buffer);
	}

	void TileEntityPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm)
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in TileEntityPacket");

		if (auto found = game->getAgent<TileEntity>(globalID)) {
			wasFound = true;
			(tileEntity = found)->decode(*game, storedBuffer);
		} else {
			std::optional<std::weak_ptr<Agent>> weak_agent;
			{
				auto lock = game->allAgents.sharedLock();
				if (auto iter = game->allAgents.find(globalID); iter != game->allAgents.end())
					weak_agent = iter->second;
			}

			wasFound = false;
			auto factory = game->registry<TileEntityFactoryRegistry>()[identifier];
			tileEntity = (*factory)();
			tileEntity->setGID(globalID);
			tileEntity->tileEntityID = identifier;
			tileEntity->setRealm(realm);
			tileEntity->init(*game);
			tileEntity->decode(*game, storedBuffer);

			if (weak_agent) {
				ERROR_("Found TileEntity " << globalID << " in allAgents, even though getAgent<TileEntity> returned null!");
				if (auto entity = std::dynamic_pointer_cast<Entity>(weak_agent->lock())) {
					nlohmann::json entity_json;
					entity->toJSON(entity_json);
					INFO_("Entity found with same global ID:\n\e[32m" << entity_json.dump() << "\e[39m");
				} else {
					if (auto agent_ptr = weak_agent->lock()) {
						Agent &agent = *agent_ptr;
						INFO_("Agent typeid: " << typeid(agent).name());
					} else {
						INFO_("Agent is expired.");
					}
				}

				nlohmann::json tile_entity_json;
				tileEntity->toJSON(tile_entity_json);
				INFO_("Tile entity data:\n\e[31m" << tile_entity_json.dump() << "\e[39m");
				assert(false);
			}

			realm->add(tileEntity);
		}

		if (tileEntity) {
			if (!wasFound)
				game->getRealm(realmID)->add(tileEntity);
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(tileEntity))
				for (InventoryID i = 0, max = has_inventory->getInventoryCount(); i < max; ++i)
					if (InventoryPtr inventory = has_inventory->getInventory(i))
						inventory->notifyOwner();
		}
	}
}
