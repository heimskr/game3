#pragma once

#include "entity/Player.h"
#include "pipes/ItemFilter.h"
#include "threading/Atomic.h"
#include "threading/Lockable.h"
#include "ui/Modifiers.h"

#include <unordered_set>

namespace Game3 {
	struct RendererContext;

	class ClientPlayer: public Player {
		private:
			Lockable<std::string> lastMessage;
			Atomic<Tick> lastMessageAge = std::numeric_limits<Tick>::max();
			std::optional<ItemFilter> copiedItemFilter;

			ClientPlayer();

		public:
			~ClientPlayer() override = default;

			static std::shared_ptr<ClientPlayer> create(Game &);

			void tick(const TickArgs &) override;
			void render(const RendererContext &) override;
			void renderLighting(const RendererContext &) override;
			void stopContinuousInteraction();
			void setContinuousInteraction(bool, Modifiers);
			void jump() override;
			const std::unordered_set<Layer> & getVisibleLayers() const;
			bool move(Direction, MovementContext) final;
			void addMoney(MoneyCount) final;
			bool removeMoney(MoneyCount) final;
			void setMoney(MoneyCount) final;
			void movedToNewChunk(const std::optional<ChunkPosition> &) override;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void sendMessage(const std::shared_ptr<Agent> &destination, const std::string &, std::any &) final;
			void setLastMessage(std::string);

			void face(Direction);

			inline void copyItemFilter(ItemFilter filter) { copiedItemFilter = std::move(filter); }
			inline const auto & pasteItemFilter() const { return copiedItemFilter; }

			using Agent::sendMessage;

		friend class Entity;
	};
}
