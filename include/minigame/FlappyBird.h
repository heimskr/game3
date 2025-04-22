#pragma once

#include "config.h"

#ifdef ENABLE_ZIP8

#include "data/Identifier.h"
#include "minigame/Minigame.h"
#include "lib/Chip8.h"

namespace Game3 {
	class Texture;

	class FlappyBird: public Minigame {
		public:
			static Identifier ID() { return {"base", "minigame/flappy_bird"}; }
			static std::string GameName() { return "Flappy Bird"; }
			std::string getGameName() const final { return GameName(); }

			using Minigame::Minigame;

			void tick(double delta) final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void reset() final;

		private:
			std::unique_ptr<Zip8> cpu;
			std::shared_ptr<Texture> display;
			bool dirty = false;
	};
}

#endif
