#pragma once

#include "ui/gl/widget/Widget.h"

#include <cstddef>
#include <string>

namespace Game3 {
	class UIContext;
	struct RendererContext;

	class Minigame: public Widget {
		public:
			int gameWidth{};
			int gameHeight{};

			Minigame(UIContext &, float scale);
			Minigame(UIContext &);

			virtual ~Minigame() = default;

			virtual std::string getGameName() const = 0;
			virtual void tick(double delta) = 0;
			virtual void reset() = 0;
			virtual void setSize(int width, int height);
			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

		protected:
			std::size_t score{};

			virtual void increaseScore(std::size_t);
	};
}
