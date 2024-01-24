#include "entity/Player.h"
#include "game/ServerGame.h"
#include "realm/Realm.h"
#include "tileentity/Stockpile.h"
#include "util/Cast.h"

namespace Game3 {
	Stockpile::Stockpile(Identifier tilename_, const Position &position_, VillageID village_id):
		TileEntity(std::move(tilename_), ID(), position_, true), villageID(village_id) {}

	bool Stockpile::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, ItemStack *, Hand) {
		if (getSide() != Side::Server)
			return false;

		VillagePtr village = getGame().toServer().getVillage(villageID);
		ServerPlayerPtr server_player = player->toServer();

		server_player->subscribeVillage(village);

		server_player->queueForMove([](const EntityPtr &entity, bool) {
			safeDynamicCast<ServerPlayer>(entity)->unsubscribeVillages();
			return true;
		});

		INFO("Village " << village->getID() << " (" << village->getName() << ") resources:");
		for (const auto &[key, value]: village->getResources())
			INFO(key << " => " << value);

		INFO("Village " << village->getID() << " (" << village->getName() << ") richness:");
		for (const auto &[key, value]: village->getRichness())
			INFO(key << " => " << value);

		return true;
	}

	void Stockpile::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << villageID;
	}

	void Stockpile::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> villageID;
	}
}
