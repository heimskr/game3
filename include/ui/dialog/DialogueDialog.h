#pragma once

#include "ui/dialog/Dialog.h"

namespace Game3 {
	class DialogueDisplay;
	class DialogueGraph;
	class DialogueNode;
	class Scroller;

	class DialogueDialog: public Dialog {
		public:
			DialogueDialog(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
			bool hidesHotbar() const final;

		private:
			std::shared_ptr<DialogueGraph> graph;
			std::shared_ptr<DialogueNode> lastNode;
			WidgetPtr dialogueWidget;

			void setDialogueWidget(WidgetPtr);
	};
}
