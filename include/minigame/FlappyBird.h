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
			std::string getName() const final { return "Flappy Bird"; }

			FlappyBird();

			void tick(UIContext &, double delta) final;
			void render(UIContext &, const RendererContext &) final;
			void setSize(int width, int height) final;
			void reset() final;

		private:
			std::unique_ptr<Zip8> cpu;
			std::shared_ptr<Texture> display;
			bool dirty = false;
	};
}

#endif
