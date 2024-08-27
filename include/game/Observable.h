#pragma once

#include "container/WeakSet.h"
#include "threading/Lockable.h"

#include <memory>

namespace Game3 {
	class Player;

	/** Some objects want to keep track of which players are "observing" them. This is useful for tile entities with inventories,
	 *  which might want to send inventory updates only to the players currently viewing its inventory. */
	class Observable {
		public:
			virtual void addObserver(const std::shared_ptr<Player> &, bool silent);
			virtual void removeObserver(const std::shared_ptr<Player> &);

			virtual ~Observable() = default;

		protected:
			Lockable<WeakSet<Player>> observers;

			Observable() = default;

			void cleanObservers();
	};
}
