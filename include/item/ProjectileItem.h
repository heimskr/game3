#pragma once

#include "entity/Player.h"
#include "entity/Projectile.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Item.h"
#include "packet/PlaySoundPacket.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	template <typename T>
	class ProjectileItem: public Item {
		public:
			ProjectileItem(ItemID id, std::string name, MoneyCount base_price, ItemCount max_count = 64):
				Item(std::move(id), std::move(name), base_price, max_count), projectileID(identifier) {}

			ProjectileItem(ItemID id, std::string name, MoneyCount base_price, Identifier projectile_id, ItemCount max_count = 64):
				Item(std::move(id), std::move(name), base_price, max_count), projectileID(std::move(projectile_id)) {}

			bool use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float> offsets) override {
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
				std::shared_ptr<T> entity = T::create(game, projectileID, velocity, 720 * (relative.column < 0? -1 : 1), place.position);
				entity->setThrower(place.player);
				entity->spawning = true;
				entity->setRealm(realm);
				entity->offset.z = player->getOffset().z;
				realm->queueEntityInit(std::move(entity), player->getPosition());
				realm->playSound(place.position, "base:sound/throw", threadContext.getPitch(1.111f));

				return true;
			}

			bool drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction action) override {
				if (action == DragAction::End) {
					return use(slot, stack, place, modifiers, offsets);
				}

				return Item::drag(slot, stack, place, modifiers, offsets, action);
			}

		private:
			Identifier projectileID;
	};
}
