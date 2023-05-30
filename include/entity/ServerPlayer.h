#pragma once

#include "entity/Player.h"
#include "threading/Lockable.h"
#include "util/WeakCompare.h"

namespace Game3 {
	class ServerPlayer: public Player {
		public:
			Lockable<WeakSet<Entity>> knownEntities;
			std::weak_ptr<RemoteClient> weakClient;

			~ServerPlayer() override = default;

			static std::shared_ptr<ServerPlayer> fromJSON(Game &, const nlohmann::json &);

			/** Returns true if the entity had to be sent. */
			bool ensureEntity(const std::shared_ptr<Entity> &);
			std::shared_ptr<RemoteClient> getClient() const;

			friend class Entity;

		private:
			ServerPlayer();
	};
}
