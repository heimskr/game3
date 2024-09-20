#pragma once

#include "graphics/Rectangle.h"
#include "threading/LockableSharedPtr.h"
#include "types/Types.h"
#include "ui/gl/Dialog.h"

#include <memory>
#include <vector>

namespace Game3 {
	class InventoryTab;
	class Tab;

	class OmniDialog: public Dialog {
		public:
			std::shared_ptr<InventoryTab> inventoryTab;
			std::shared_ptr<Tab> craftingTab;
			std::shared_ptr<Tab> settingsTab;
			std::shared_ptr<Tab> activeTab;

			OmniDialog(UIContext &);

			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
			void onClose() final;
			bool click(int button, int x, int y) final;
			bool dragStart(int x, int y) final;
			bool dragUpdate(int x, int y) final;
			bool dragEnd(int x, int y) final;
			bool scroll(float x_delta, float y_delta, int x, int y) final;
			void updateModule();

			template <typename T>
			size_t removeTab() {
				auto lock = tabs.uniqueLock();
				return std::erase_if(tabs, &tabMatcher<T>);
			}

			template <typename T>
			bool hasTab() const {
				auto lock = tabs.sharedLock();
				return std::ranges::any_of(tabs, &tabMatcher<T>);
			}

			template <typename T, typename... Args>
			void addTab(Args &&...args) {
				auto lock = tabs.uniqueLock();
				tabs.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
			}

		private:
			Lockable<std::vector<std::shared_ptr<Tab>>> tabs;
			std::vector<Rectangle> tabRectangles;

			template <typename T>
			static bool tabMatcher(const std::shared_ptr<Tab> &tab) {
				return dynamic_cast<const T *>(tab.get()) != nullptr;
			}
	};
}
