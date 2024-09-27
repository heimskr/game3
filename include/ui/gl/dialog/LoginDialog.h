#pragma once

#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/Types.h"

#include <sigc++/sigc++.h>

#include <memory>

namespace Game3 {
	class Box;
	class TextInput;

	class LoginDialog: public DraggableDialog {
		public:
			sigc::signal<void(const UString &username, const UString &display_name)> signalSubmit;

			LoginDialog(UIContext &);

			void render(const RendererContext &) override;

			void init() override;

			static std::shared_ptr<LoginDialog> create(UIContext &, UString text, ButtonsType = ButtonsType::Okay);

		private:
			WidgetPtr child;
			std::shared_ptr<TextInput> usernameInput;
			std::shared_ptr<TextInput> displayNameInput;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Box> buttonBox;

			void submit(bool);
			std::function<bool(Widget &, int, int, int)> makeSubmit(bool);
	};
}
