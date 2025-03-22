#pragma once

#include "graphics/Rectangle.h"
#include "threading/LockableSharedPtr.h"
#include "ui/gl/tab/Tab.h"

namespace Game3 {
	class InventoryModule;
	class Module;
	class Scroller;

	class InventoryTab: public Tab {
		public:
			InventoryTab(UIContext &, float scale);

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;
			bool click(int button, int x, int y, Modifiers) final;
			bool dragStart(int x, int y) final;
			bool dragEnd(int x, int y) final;
			bool scroll(float x_delta, float y_delta, int x, int y, Modifiers) final;

			void setModule(std::shared_ptr<Module>);
			Module * getModule(std::shared_lock<DefaultMutex> &);
			Module * getModule(std::unique_lock<DefaultMutex> &);
			void removeModule();

		private:
			LockableSharedPtr<Module> activeModule;
			std::shared_ptr<InventoryModule> playerInventoryModule;
			std::shared_ptr<Scroller> playerScroller;
			std::shared_ptr<Scroller> moduleScroller;

			std::shared_ptr<Scroller> makePlayerScroller();
			std::shared_ptr<Scroller> makeModuleScroller();

			bool onSlotClick(Slot, Modifiers);
	};
}
