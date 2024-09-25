#pragma once

#include "types/UString.h"
#include "ui/gl/dialog/Dialog.h"

#include <memory>

namespace Game3 {
	class Icon;

	class BaseDraggableDialog: public Dialog {
		public:
			BaseDraggableDialog(UIContext &, int width, int height);

			void render(const RendererContext &) override;
			Rectangle getPosition() const override;
			void init() override;
			bool click(int button, int x, int y) override;

			virtual float getTitleScale() const = 0;
			virtual const UString & getTitle() const = 0;

		protected:
			Rectangle position;
			Rectangle titleRectangle;
			std::shared_ptr<Icon> closeButton;
	};

	class DraggableDialog: public BaseDraggableDialog {
		public:
			using BaseDraggableDialog::BaseDraggableDialog;

			float getTitleScale() const override;
			const UString & getTitle() const override;
			void setTitle(UString);


		private:
			UString title;
	};
}
