#include "entity/Player.h"
#include "game/ServerGame.h"
#include "packet/OpenVillageTradePacket.h"
#include "packet/VillageUpdatePacket.h"
#include "realm/Realm.h"
#include "tileentity/Stockpile.h"
#include "ui/module/VillageTradeModule.h"
#include "util/Cast.h"

namespace Game3 {
	Stockpile::Stockpile(Identifier tilename_, const Position &position_, VillageID village_id):
		TileEntity(std::move(tilename_), ID(), position_, true), villageID(village_id) {}

	bool Stockpile::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		if (getSide() != Side::Server)
			return false;

		GamePtr game = getGame();

		VillagePtr village = game->getVillage(villageID);
		ServerPlayerPtr server_player = player->toServer();

		server_player->subscribeVillage(village);

		RealmPtr realm = game->getRealm(village->getRealmID());
		player->notifyOfRealm(*realm);
		player->send(VillageUpdatePacket(*village));
		player->send(OpenVillageTradePacket(villageID));

		server_player->queueForMove([](const EntityPtr &entity, bool) {
			safeDynamicCast<ServerPlayer>(entity)->unsubscribeVillages();
			return true;
		});

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
