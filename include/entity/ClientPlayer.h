#pragma once

#include "entity/Player.h"

namespace Game3 {
	class ClientPlayer: public Player {
		public:
			~ClientPlayer() override = default;

			void render(SpriteRenderer &, TextRenderer &) override;

			friend class Entity;

		private:
			ClientPlayer();
	};
}
