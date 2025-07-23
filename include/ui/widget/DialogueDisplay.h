#pragma once

#include "game/Dialogue.h"
#include "ui/widget/Box.h"
#include "ui/widget/Forward.h"

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

	class DialogueDisplay: public Box {
		public:
			DialogueDisplay(UIContext &, float selfScale, DialogueGraphPtr graph);

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) final;

			bool getStillOpen() const;
			TexturePtr getFaceTexture() const;

		private:
			DialogueGraphPtr graph;
			DialogueNodePtr lastNode;
			LabelPtr mainText;
			BoxPtr optionBox;
			TexturePtr faceTexture;
			size_t selectedOptionIndex = 0;

			void resetOptions(const DialogueNodePtr &);
			void selectOption(size_t index);
			void activateOption();
	};
}
