#pragma once

#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

#include <sigc++/sigc++.h>

#include <filesystem>
#include <memory>

namespace Game3 {
	class Box;
	class Scroller;

	class WorldsDialog final: public DraggableDialog {
		public:
			std::filesystem::path currentPath;
			sigc::signal<void(const std::filesystem::path &)> signalSubmit;

			WorldsDialog(UIContext &, float selfScale);

			void init() override;
			void render(const RendererContext &) override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;
			void rescale(float new_scale) final;

			void submit(const std::filesystem::path &world_path);
			void populate();

			static std::shared_ptr<WorldsDialog> create(UIContext &, UString text);

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Box> vbox;
			bool showHidden = true;
	};
}
