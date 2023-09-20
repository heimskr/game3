#pragma once

#include "entity/Player.h"
#include "threading/Atomic.h"
#include "threading/Lockable.h"
#include "ui/Modifiers.h"

#include <unordered_set>

namespace Game3 {
	class ClientPlayer: public Player {
		private:
			Lockable<std::string> lastMessage;
			Atomic<float> lastMessageAge = std::numeric_limits<float>::infinity();

			ClientPlayer();

		public:
			~ClientPlayer() override = default;

			static std::shared_ptr<ClientPlayer> create(Game &);

			void tick(Game &, float delta) override;
			void render(SpriteRenderer &, TextRenderer &) override;
			void stopContinuousInteraction();
			void setContinuousInteraction(bool, Modifiers);
			void jump() override;
			const std::unordered_set<Layer> & getVisibleLayers() const;
			bool move(Direction, MovementContext) final;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void sendMessage(const std::shared_ptr<Agent> &destination, const std::string &, std::any &) final;

			void setLastMessage(std::string);

			using Agent::sendMessage;

		friend class Entity;
	};
}
