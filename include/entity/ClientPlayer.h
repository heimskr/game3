#pragma once

#include "entity/Player.h"
#include "ui/Modifiers.h"

#include <unordered_set>

namespace Game3 {
	class ClientPlayer: public Player {
		public:
			~ClientPlayer() override = default;

			static std::shared_ptr<ClientPlayer> create(Game &);

			void render(SpriteRenderer &, TextRenderer &) override;
			void stopContinuousInteraction();
			void setContinuousInteraction(bool, Modifiers);
			void jump() override;
			const std::unordered_set<Layer> & getVisibleLayers() const;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void sendMessage(const std::shared_ptr<Agent> &destination, const std::string &, std::any &) final;

			using Agent::sendMessage;

		private:
			ClientPlayer();

		friend class Entity;
	};
}
