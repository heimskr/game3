#pragma once

#include "dialogue/Dialogue.h"

#include <memory>
#include <string>

namespace Game3 {
	class ItemSlot;
	class ItemStack;

	class RhosumShopNode: public DialogueNode {
		public:
			RhosumShopNode(DialogueGraph &parent, std::string name, TexturePtr faceOverride = {});

			bool render(UIContext &, const RendererContext &) final;

		private:
			std::shared_ptr<ItemSlot> lampOil;
			std::shared_ptr<ItemSlot> rope;
			std::shared_ptr<ItemSlot> bomb;

	};
}
