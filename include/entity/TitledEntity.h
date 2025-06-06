#pragma once

#include "entity/Entity.h"
#include "types/UString.h"

#include <limits>
#include <string>

namespace Game3 {
	class ClientGame;

	class TitledEntity: public virtual Entity {
		public:
			Lockable<UString> lastMessage;
			Atomic<Tick> lastMessageAge = std::numeric_limits<Tick>::max();

			void renderUpper(const RendererContext &) override;

			virtual float getTitleVerticalOffset() const;
			virtual Tick getMaxMessageAge(ClientGame &) const;
			virtual UString getDisplayName() = 0;
	};
}
