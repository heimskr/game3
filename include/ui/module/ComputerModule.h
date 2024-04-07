#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <vte/vte.h>

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Computer;

	class ComputerModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/computer"}; }

			/** The std::any argument is expected to hold an AgentPtr to a Computer tile entity. */
			ComputerModule(std::shared_ptr<ClientGame>, const std::any &);
			ComputerModule(std::shared_ptr<ClientGame>, const AgentPtr &);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Computer> computer;
			Gtk::Widget *terminal = nullptr;
			VteTerminal *vte = nullptr;
			Gtk::Entry entry;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void runScript(const std::string &);
	};
}
