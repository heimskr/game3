#pragma once

#include "game/ClientInventory.h"
#include "types/Types.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/module/GeneticAnalysisModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/MicroscopeModule.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/module/MultiModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/UIContext.h"
#include "ui/MainWindow.h"
#include "util/Cast.h"

#include <any>
#include <map>
#include <string>

namespace Game3 {
	template <Slot S, Substance... ExtraSubstances>
	class MicroscopeModule: public Module {
		private:
			using Submodule = MultiModule<Substance::Item, ExtraSubstances...>;

		public:
			static Identifier ID() { return {"base", std::format("module/microscope/{}{}", S, Submodule::getSuffix())}; }

			MicroscopeModule(UIContext &ui, const ClientGamePtr &game, const std::any &argument):
				MicroscopeModule(ui, game, std::dynamic_pointer_cast<InventoriedTileEntity>(std::any_cast<AgentPtr>(argument))) {}

			MicroscopeModule(UIContext &ui, const ClientGamePtr &game, std::shared_ptr<InventoriedTileEntity> tile_entity):
				Module(ui, game),
				tileEntity(std::move(tile_entity)),
				multiModule(std::make_shared<Submodule>(ui, game, std::static_pointer_cast<Agent>(tileEntity))),
				geneticAnalysisModule(std::make_shared<GeneticAnalysisModule>(ui)) {}

			Identifier getID() const final {
				return ID();
			}

			void init() final {
				multiModule->init();
				geneticAnalysisModule->init();

				vbox = std::make_shared<Box>(ui, scale);
				vbox->setHorizontalExpand(true);

				header = std::make_shared<Label>(ui, scale);
				header->setText(tileEntity->getName());
				header->insertAtEnd(vbox);

				multiModule->insertAtEnd(vbox);
				geneticAnalysisModule->insertAtEnd(vbox);

				vbox->insertAtEnd(shared_from_this());

				initialized = true;
			}

			void reset() final {
				multiModule->reset();
				updateResults();
			}

			void update() final {
				multiModule->update();
				updateResults();
			}

			void updateResults() {
				if (!tileEntity || !initialized)
					return;

				auto inventory = tileEntity->getInventory(0);
				auto lock = inventory->sharedLock();
				geneticAnalysisModule->update((*inventory)[S]);
			}

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final {
				if (name == "TileEntityRemoved") {

					if (source && source->getGID() == tileEntity->getGID()) {
						multiModule->handleMessage(source, name, data);
						MainWindow &window = getGame()->getWindow();
						window.queue([&window] { window.removeModule(); });
					}

				} else if (name == "GetAgentGID") {

					return Buffer{Side::Client, tileEntity->getGID()};

				} else {

					return multiModule->handleMessage(source, name, data);

				}

				return std::nullopt;
			}

			void setInventory(std::shared_ptr<ClientInventory> inventory) final {
				multiModule->setInventory(std::move(inventory));
				updateResults();
			}

			// std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final {
			// 	return multiModule->getPrimaryInventoryModule();
			// }

			void render(const RendererContext &renderers, float x, float y, float width, float height) final {
				Widget::render(renderers, x, y, width, height);
				vbox->render(renderers, x, y, width, height);
			}

			SizeRequestMode getRequestMode() const final {
				return vbox->getRequestMode();
			}

			void measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) final {
				return vbox->measure(renderers, orientation, for_width, for_height, minimum, natural);
			}

		private:
			std::shared_ptr<InventoriedTileEntity> tileEntity;
			std::shared_ptr<Submodule> multiModule;
			std::shared_ptr<GeneticAnalysisModule> geneticAnalysisModule;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Label> header;
			bool initialized = false;
	};
}
