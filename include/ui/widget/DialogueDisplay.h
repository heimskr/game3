#pragma once

#include "dialogue/Dialogue.h"
#include "ui/widget/Box.h"
#include "ui/widget/Forward.h"
#include "ui/widget/Scroller.h"

#include <memory>

namespace Game3 {
	class Label;

	class DialogueRow: public Box {
		public:
			DialogueRow(UIContext &, float selfScale, DialogueOption);

			void init() override;

			void setActive(bool);

		private:
			DialogueOption option;
			LabelPtr indicator;
			LabelPtr text;
	};

	class DialogueDisplay: public Scroller {
		public:
			DialogueDisplay(UIContext &, float selfScale, DialogueNodePtr node);

			void init() override;
			void render(const RendererContext &, float x, float y, float width, float height) override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;

			TexturePtr getFaceTexture() const;
			DialogueGraphPtr getGraph() const;

		private:
			DialogueNodePtr node;
			BoxPtr mainBox;
			LabelPtr mainText;
			BoxPtr optionBox;
			TexturePtr faceTexture;
			size_t selectedOptionIndex = 0;

			void selectOption(size_t index);
			void activateOption();
			Rectangle getPosition() const;
	};
}
