#pragma once

#include "types/UString.h"
#include "ui/dialog/Dialog.h"

#include <sigc++/sigc++.h>

#include <memory>
#include <optional>

namespace Game3 {
	class Icon;

	class BaseDraggableDialog: public Dialog {
		public:
			sigc::signal<void()> signalDismiss;

			BaseDraggableDialog(UIContext &, float selfScale, int width, int height);

			void render(const RendererContext &) override;
			Rectangle getPosition() const override;
			Rectangle getInnerRectangle() const;
			void init() override;
			bool click(int button, int x, int y, Modifiers) override;
			bool dragStart(int x, int y) override;
			bool dragUpdate(int x, int y) override;
			bool dragEnd(int x, int y, double) override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;
			bool blocksMouse(int x, int y, bool is_drag_update) const override;

			virtual float getTitleScale() const = 0;
			virtual const UString & getTitle() const = 0;

			void recenter();

			static int getEffectiveWidth(int content_width, float scale);
			static int getEffectiveHeight(int content_height, float scale);

		protected:
			Rectangle position;
			Rectangle titleRectangle;
			Rectangle bodyRectangle;
			std::shared_ptr<Icon> closeButton;
			std::optional<std::pair<int, int>> dragOffset;
			int dialogWidth{};
			int dialogHeight{};
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
