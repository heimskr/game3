#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class ArcadeMachine: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/arcade_machine"}; }
			Identifier minigameName;
			int gameWidth{};
			int gameHeight{};

			std::string getName() const override { return "Arcade Machine"; }

			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool mouseOver() final;
			void mouseOut() final;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;

		protected:
			ArcadeMachine() = default;
			ArcadeMachine(Identifier tilename, Position position, Identifier minigame_name, int game_width, int game_height);

		friend class TileEntity;
	};
}
