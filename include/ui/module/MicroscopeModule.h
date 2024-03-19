#pragma once

#include "types/Types.h"
#include "ui/module/GeneticAnalysisModule.h"
#include "ui/module/Module.h"
#include "ui/module/MultiModule.h"
#include "ui/MainWindow.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class InventoriedTileEntity;
	class InventoryModule;

	template <Slot S, Substance... ExtraSubstances>
	class MicroscopeModule: public Module {
		private:
			using Submodule = MultiModule<Substance::Item, ExtraSubstances...>;

		public:
			static Identifier ID() { return {"base", "module/microscope_" + Submodule::getSuffix()}; }

			MicroscopeModule(ClientGamePtr game_, const std::any &argument):
				MicroscopeModule(game_, std::dynamic_pointer_cast<InventoriedTileEntity>(std::any_cast<AgentPtr>(argument))) {}

			MicroscopeModule(ClientGamePtr game_, std::shared_ptr<InventoriedTileEntity> tile_entity):
			game(std::move(game_)),
			tileEntity(std::move(tile_entity)),
			multiModule(std::make_shared<Submodule>(game, std::static_pointer_cast<Agent>(tileEntity))) {
				vbox.set_hexpand();
				header.set_text(tileEntity->getName());
				header.set_margin(10);
				header.set_xalign(0.5);
				vbox.append(header);
				vbox.append(multiModule->getWidget());
				vbox.append(geneticAnalysisModule.getWidget());
			}

			Identifier getID() const final {
				return ID();
			}

			Gtk::Widget & getWidget() final {
				return vbox;
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
				if (!tileEntity)
					return;

				auto inventory = tileEntity->getInventory(0);
				auto lock = inventory->sharedLock();
				geneticAnalysisModule.update((*inventory)[S]);
			}

			void onResize(int width) final {
				multiModule->onResize(width);
			}

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final {
				if (name == "TileEntityRemoved") {

					if (source && source->getGID() == tileEntity->getGID()) {
						multiModule->handleMessage(source, name, data);
						MainWindow &window = game->getWindow();
						window.queue([&window] { window.removeModule(); });
					}

				} else if (name == "GetAgentGID") {

					return Buffer{tileEntity->getGID()};

				} else {

					return multiModule->handleMessage(source, name, data);

				}

				return std::nullopt;
			}

			void setInventory(std::shared_ptr<ClientInventory> inventory) final {
				multiModule->setInventory(std::move(inventory));
				updateResults();
			}

			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final {
				return multiModule->getPrimaryInventoryModule();
			}

		private:
			ClientGamePtr game;
			std::shared_ptr<InventoriedTileEntity> tileEntity;
			std::shared_ptr<Submodule> multiModule;
			GeneticAnalysisModule geneticAnalysisModule;
			Gtk::Label header;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
