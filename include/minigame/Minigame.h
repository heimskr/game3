#pragma once

#include <cstddef>

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Minigame {
		public:
			int gameWidth{};
			int gameHeight{};

			virtual ~Minigame() = default;

			virtual void tick(UIContext &, double delta) = 0;
			virtual void render(UIContext &, const RendererContext &) = 0;
			virtual void setSize(float width, float height) = 0;

		protected:
			std::size_t score{};

			virtual void increaseScore(std::size_t);
	};
}
