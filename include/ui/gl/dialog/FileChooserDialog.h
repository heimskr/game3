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

	class FileChooserDialog: public DraggableDialog {
		public:
			std::filesystem::path currentPath;
			sigc::signal<void(const std::filesystem::path &)> signalSubmit;

			FileChooserDialog(UIContext &, float selfScale, UString title, int width, int height);

			void init() override;
			void render(const RendererContext &) override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;
			void rescale(float new_scale) override;

			void submit(const std::filesystem::path &world_path);
			void populate();

		protected:
			int baseWidth{};
			int baseHeight{};

			virtual bool filter(const std::filesystem::directory_entry &) const;

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Box> vbox;
			bool showHidden = true;
	};
}
