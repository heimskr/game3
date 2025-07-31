#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class DialogueNode;

	class Speaker: public virtual Entity {
		public:
			virtual void dialogueSelected(const std::shared_ptr<DialogueNode> &) = 0;
			virtual TexturePtr getFaceTexture() = 0;

		protected:
			Speaker();
	};

	using SpeakerPtr = std::shared_ptr<Speaker>;
}
