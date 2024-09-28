#pragma once

#include "types/UString.h"
#include "ui/gl/dialog/Dialog.h"

namespace Game3 {
	class Box;

	class ChatDialog: public Dialog {
		public:
			ChatDialog(UIContext &);

			void init() override;
			void render(const RendererContext &) override;
			Rectangle getPosition() const override;
			bool click(int button, int x, int y) override;
			bool scroll(float x_delta, float y_delta, int x, int y) override;
			void onFocus() override;
			void onBlur() override;

			void addMessage(UString);

		private:
			std::shared_ptr<Box> messageBox;
	};
}
