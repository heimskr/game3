#pragma once

#include "entity/Player.h"
#include "threading/Lockable.h"
#include "container/WeakSet.h"

namespace Game3 {
	class GenericClient;
	class Village;

	class ServerPlayer: public Player {
		public:
			Lockable<WeakSet<Entity>> knownEntities;
			std::weak_ptr<GenericClient> weakClient;
			bool inventoryUpdated = false;

			~ServerPlayer() override;

			static std::shared_ptr<ServerPlayer> create(const std::shared_ptr<Game> &);
			static std::shared_ptr<ServerPlayer> fromJSON(const std::shared_ptr<Game> &, const boost::json::value &);
			static std::shared_ptr<ServerPlayer> fromBuffer(const std::shared_ptr<Game> &, Buffer &);

			/** Returns true if the entity had to be sent. */
			bool ensureEntity(const std::shared_ptr<Entity> &);
			std::shared_ptr<GenericClient> getClient() const;

			void tick(const TickArgs &) final;
			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void movedToNewChunk(const std::optional<ChunkPosition> &) override;

			void addMoney(MoneyCount) final;
			bool removeMoney(MoneyCount) final;
			void broadcastMoney() final;

			void kill() override;

			void unsubscribeVillages();
			void subscribeVillage(const std::shared_ptr<Village> &);

			void showText(const UString &text, const UString &name) final;

		private:
			std::weak_ptr<Village> subscribedVillage;

			ServerPlayer();

		friend class Entity;
	};
}
