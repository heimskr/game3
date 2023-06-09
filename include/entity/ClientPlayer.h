#pragma once

#include "entity/Player.h"
#include "ui/Modifiers.h"

namespace Game3 {
	class ClientPlayer: public Player {
		public:
			~ClientPlayer() override = default;

			static std::shared_ptr<ClientPlayer> create(Game &);

			void render(SpriteRenderer &, TextRenderer &) override;
			void stopContinuousInteraction();
			void setContinuousInteraction(bool, Modifiers);
			void jump() override;

		private:
			ClientPlayer();

		friend class Entity;
	};
}
