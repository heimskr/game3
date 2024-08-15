#include "entity/Player.h"
#include "entity/Projectile.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/ProjectileItem.h"
#include "packet/PlaySoundPacket.h"
#include "realm/Realm.h"

#include <cmath>

namespace Game3 {
	ProjectileItem::ProjectileItem(ItemID id, std::string name, MoneyCount base_price, ItemCount max_count):
		Item(std::move(id), std::move(name), base_price, max_count), projectileID(identifier) {}

	ProjectileItem::ProjectileItem(ItemID id, std::string name, MoneyCount base_price, Identifier projectile_id, ItemCount max_count):
		Item(std::move(id), std::move(name), base_price, max_count), projectileID(std::move(projectile_id)) {}

	bool ProjectileItem::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float> offsets) {
		PlayerPtr player = place.player;
		GamePtr game = player->getGame();

		assert(game->getSide() == Side::Server);

		player->getInventory(0)->decrease(stack, slot, 1, true);

		constexpr static double scale = 1.5;
		const Position relative = place.position - player->getPosition();
		Vector3 velocity(relative.column + (0.5 - offsets.first), relative.row + (0.5 - offsets.second), 16.0);
		velocity.x *= scale;
		velocity.y *= scale;
		velocity.z /= scale;

		RealmPtr realm = place.realm;
		EntityPtr entity = Projectile::create(game, projectileID, velocity, 720 * (relative.column < 0? -1 : 1), 5);
		entity->spawning = true;
		entity->setRealm(realm);
		entity->offset.z = player->getOffset().z;
		realm->queueEntityInit(std::move(entity), player->getPosition());

		realm->getPlayers().withShared([&, origin = player->getPosition()](const WeakSet<Player> &set) {
			for (const auto &weak_player: set)
				if (auto player = weak_player.lock())
					player->send(PlaySoundPacket("base:sound/throw", origin));
		});

		return true;
	}
}
