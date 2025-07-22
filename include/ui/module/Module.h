#pragma once

#include "data/Identifier.h"
#include "net/Buffer.h"
#include "ui/widget/Widget.h"
#include "ui/widget/ChildDependentExpandingWidget.h"

#include <any>
#include <string>

namespace Game3 {
	class Agent;
	class ClientInventory;
	class InventoryModule;

	class Module: public ChildDependentExpandingWidget<Widget> {
		public:
			Module(UIContext &, float selfScale, std::weak_ptr<ClientGame>);
			Module(UIContext &, float selfScale);

			virtual ~Module() = default;

			virtual Identifier getID() const = 0;
			virtual void reset();
			virtual void update();
			virtual void setInventory(std::shared_ptr<ClientInventory>);
			virtual std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &);
			virtual std::shared_ptr<InventoryModule> getPrimaryInventoryModule();
			/** Returns false if the default shift click behavior should occur, or true otherwise. */
			virtual bool handleShiftClick(std::shared_ptr<Inventory> /* source_inventory */, Slot);

			ClientGamePtr getGame() const;

		protected:
			std::weak_ptr<ClientGame> weakGame;
	};

	using ModulePtr = std::shared_ptr<Module>;
}
