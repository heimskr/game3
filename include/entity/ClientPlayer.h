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

			void handleMessage(Agent &source, const std::string &name, Buffer &data) final;
			void sendBuffer(Agent &, const std::string &, Buffer &) final;

		private:
			ClientPlayer();

		friend class Entity;
	};
}
