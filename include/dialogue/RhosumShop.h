#pragma once

#include "dialogue/Dialogue.h"
#include "ui/widget/FullscreenWidget.h"

#include <memory>
#include <string>

namespace Game3 {
	class RhosumShopWidget: public FullscreenWidget {
		public:
			RhosumShopWidget(UIContext &, float selfScale);

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;

		private:
			TexturePtr talksprite;
			ItemSlotPtr lampOil;
			ItemSlotPtr rope;
			ItemSlotPtr bomb;
	};

	class RhosumShopNode: public DialogueNode {
		public:
			using DialogueNode::DialogueNode;

			WidgetPtr getWidget(UIContext &) final;
	};
}
