#pragma once

#include "FTLabel.h"

#include "entity/Player.h"

namespace Game3 {
	class ClientPlayer: public Player {
		public:
			~ClientPlayer() override = default;

			void render(SpriteRenderer &) override;

			friend class Entity;

		private:
			std::unique_ptr<FTLabel> label;

			ClientPlayer();

			int backbufferWidth  = -1;
			int backbufferHeight = -1;
	};
}
