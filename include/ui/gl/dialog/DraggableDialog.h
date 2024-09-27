#pragma once

#include "types/UString.h"
#include "ui/gl/dialog/Dialog.h"

#include <sigc++/sigc++.h>

#include <memory>
#include <optional>

namespace Game3 {
	class Icon;

	class BaseDraggableDialog: public Dialog {
		public:
			sigc::signal<void()> signalDismiss;

			BaseDraggableDialog(UIContext &, int width, int height);

			void render(const RendererContext &) override;
			Rectangle getPosition() const override;
			void init() override;
			bool click(int button, int x, int y) override;
			bool dragStart(int x, int y) override;
			bool dragUpdate(int x, int y) override;
			bool dragEnd(int x, int y) override;
			bool keyPressed(uint32_t character, Modifiers) override;

			virtual float getTitleScale() const = 0;
			virtual const UString & getTitle() const = 0;

		protected:
			Rectangle position;
			Rectangle titleRectangle;
			Rectangle bodyRectangle;
			std::shared_ptr<Icon> closeButton;
			std::optional<std::pair<int, int>> dragOffset;
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
