#pragma once

#include "dialogue/Dialogue.h"
#include "types/Types.h"
#include "ui/widget/Forward.h"
#include "ui/widget/FullscreenWidget.h"
#include "util/Noticer.h"

#include <memory>
#include <string>

namespace Game3 {
	class RhosumShopNode;

	/** Selects option 0 to exit, option 1 for not enough funds, option 2 for successful purchase. */
	class RhosumShopWidget: public FullscreenWidget {
		public:
			RhosumShopWidget(UIContext &, float selfScale, const std::shared_ptr<RhosumShopNode> &parent);

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;

		private:
			std::weak_ptr<RhosumShopNode> weakParent;
			TexturePtr talksprite;
			LabelPtr moneyLabel;
			IconPtr exitIcon;
			ItemSlotPtr lampOil;
			ItemSlotPtr rope;
			ItemSlotPtr bomb;

			Noticer<MoneyCount> moneyNoticer;
	};

	class RhosumShopNode: public DialogueNode {
		public:
			using DialogueNode::DialogueNode;

			WidgetPtr getWidget(UIContext &) final;
	};
}
