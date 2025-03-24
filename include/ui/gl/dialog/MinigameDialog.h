#pragma once

#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "minigame/Minigame.h"
#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

#include <chrono>

namespace Game3 {
	class Minigame;

	class MinigameDialog: public DraggableDialog {
		public:
			int width{};
			int height{};

			MinigameDialog(UIContext &ui, float selfScale, std::shared_ptr<Minigame> minigame, int width, int height);
			void init() override;
			void render(const RendererContext &renderers) override;
			void onClose() override;
			void onFocus() override;
			void onBlur() override;

		protected:
			std::shared_ptr<Minigame> minigame;
			std::chrono::system_clock::time_point lastTime;
	};
}
