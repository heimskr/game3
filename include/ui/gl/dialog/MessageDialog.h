#pragma once

#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

#include <sigc++/sigc++.h>

#include <memory>

namespace Game3 {
	class Box;

	class MessageDialog: public DraggableDialog {
		public:
			sigc::signal<void(bool response)> onSubmit;

			MessageDialog(UIContext &, int width, int height, ButtonsType = ButtonsType::Okay);

			void render(const RendererContext &) override;

			void init() override;
			bool click(int button, int x, int y) override;
			bool dragStart(int x, int y) override;
			bool dragUpdate(int x, int y) override;
			bool dragEnd(int x, int y) override;
			bool scroll(float x_delta, float y_delta, int x, int y) override;
			bool keyPressed(uint32_t character, Modifiers) override;

			void setChild(WidgetPtr);

			static std::shared_ptr<MessageDialog> create(UIContext &, UString text, ButtonsType = ButtonsType::Okay);

		private:
			ButtonsType buttonsType = ButtonsType::Okay;
			WidgetPtr child;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Box> buttonBox;

			void submit(bool);
			std::function<bool(Widget &, int, int, int)> makeSubmit(bool);
	};
}
