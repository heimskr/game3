#pragma once

#include <cstddef>
#include <string>

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Minigame {
		public:
			int gameWidth{};
			int gameHeight{};

			virtual ~Minigame() = default;

			virtual std::string getName() const = 0;
			virtual void tick(UIContext &, double delta) = 0;
			virtual void reset() = 0;
			virtual void render(UIContext &, const RendererContext &) = 0;
			virtual void setSize(float width, float height) = 0;

		protected:
			std::size_t score{};

			virtual void increaseScore(std::size_t);
	};
}
