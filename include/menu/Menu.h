#pragma once

struct NVGcontext;

namespace Game3 {
	class Canvas;
	class Game;

	enum class MenuType {Inventory};

	struct Menu {
		virtual ~Menu() = default;
		virtual void render(Game &, Canvas &, NVGcontext *) = 0;
		virtual MenuType getType() const = 0;
	};
}
