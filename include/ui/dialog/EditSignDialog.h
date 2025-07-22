#pragma once

#include "ui/dialog/DraggableDialog.h"
#include "ui/widget/Widget.h"
#include "ui/Types.h"

#include <sigc++/sigc++.h>

#include <memory>

namespace Game3 {
	class Box;
	class Grid;
	class TextInput;

	class EditSignDialog: public DraggableDialog {
		public:
			sigc::signal<void(UString tilename, UString contents)> onSubmit;

			EditSignDialog(UIContext &, float selfScale, int width, int height, UString initialTilename, UString initialContents);

			void render(const RendererContext &) override;

			void init() override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;
			void childResized(const WidgetPtr &child, Orientation, int new_width, int new_height) override;

		private:
			UString initialTilename;
			UString initialContents;
			std::shared_ptr<TextInput> tilenameInput;
			std::shared_ptr<TextInput> contentsInput;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Box> buttonBox;
			std::shared_ptr<Grid> grid;
	};
}
