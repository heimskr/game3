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
		globalID(tileEntity->globalID),
		realmID(tileEntity->realmID) {}

	void TileEntityPacket::decode(Game &game, Buffer &buffer) {
		buffer >> globalID >> identifier >> realmID;
		assert(globalID != static_cast<GlobalID>(-1));
		assert(globalID != static_cast<GlobalID>(0));

		RealmPtr realm = game.tryRealm(realmID);
		if (!realm)
			throw PacketError("Couldn't find realm " + std::to_string(realmID) + " in TileEntityPacket");

		if (auto found = game.getAgent<TileEntity>(globalID)) {
			wasFound = true;
			(tileEntity = found)->decode(game, buffer);
		} else {
			std::optional<std::weak_ptr<Agent>> weak_agent;
			{
				auto lock = game.allAgents.sharedLock();
				if (auto iter = game.allAgents.find(globalID); iter != game.allAgents.end())
					weak_agent = iter->second;
			}

			wasFound = false;
			auto factory = game.registry<TileEntityFactoryRegistry>()[identifier];
			tileEntity = (*factory)(game);
			tileEntity->globalID = globalID;
			tileEntity->tileEntityID = identifier;
			tileEntity->decode(game, buffer);

			if (weak_agent) {
				ERROR("Found TileEntity " << globalID << " in allAgents, even though getAgent<TileEntity> returned null!");
				if (auto entity = std::dynamic_pointer_cast<Entity>(weak_agent->lock())) {
					nlohmann::json entity_json;
					entity->toJSON(entity_json);
					INFO("Entity found with same global ID:\n\e[32m" << entity_json.dump() << "\e[39m");
				} else {
					if (auto agent_ptr = weak_agent->lock()) {
						Agent &agent = *agent_ptr;
						INFO("Agent typeid: " << typeid(agent).name());
					} else {
						INFO("Agent is expired.");
					}
				}

				nlohmann::json tile_entity_json;
				tileEntity->toJSON(tile_entity_json);
				INFO("Tile entity data:\n\e[31m" << tile_entity_json.dump() << "\e[39m");
				assert(false);
			}

			tileEntity->init(game);
			realm->add(tileEntity);
		}
	}

	void TileEntityPacket::encode(Game &game, Buffer &buffer) const {
		assert(tileEntity);
		buffer << globalID << identifier << realmID;
		tileEntity->encode(game, buffer);
	}

	void TileEntityPacket::handle(ClientGame &game) {
		if (tileEntity) {
			if (!wasFound)
				game.getRealm(realmID)->add(tileEntity);
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(tileEntity))
				has_inventory->inventory->notifyOwner();
		}
	}
}
