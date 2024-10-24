#pragma once

#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

#include <chrono>

namespace Game3 {
	template <typename MG, int W, int H>
	class MinigameDialog: public DraggableDialog {
		public:
			MinigameDialog(UIContext &ui):
				DraggableDialog(ui, BaseDraggableDialog::getEffectiveWidth(W, UI_SCALE), BaseDraggableDialog::getEffectiveHeight(H, UI_SCALE)) {
					setTitle(std::string(MG::getName()));
				}

			void init() override {
				DraggableDialog::init();

				lastTime = std::chrono::system_clock::now();
				minigame = std::make_shared<MG>();
				minigame->setSize(W, H);
				minigame->reset();
			}

			void render(const RendererContext &renderers) override {
				DraggableDialog::render(renderers);

				if (!minigame) {
					return;
				}

				auto now = std::chrono::system_clock::now();
				double delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastTime).count() / 1e9;
				lastTime = now;
				minigame->tick(ui, delta);

				auto saver = ui.scissorStack.pushAbsolute(getInnerRectangle(), renderers);
				minigame->render(ui, renderers);
			}

		protected:
			std::shared_ptr<MG> minigame;
			std::chrono::system_clock::time_point lastTime;
	};
}
