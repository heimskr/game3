#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "game/ServerGame.h"
#include "tileentity/Computer.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/Util.h"
#include "ui/module/ComputerModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"
#include "util/FS.h"

namespace Game3 {
	ComputerModule::ComputerModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		ComputerModule(std::move(game_), std::any_cast<AgentPtr>(argument)) {}

	ComputerModule::ComputerModule(std::shared_ptr<ClientGame> game_, const AgentPtr &agent):
	game(std::move(game_)),
	computer(std::dynamic_pointer_cast<Computer>(agent)) {
		terminal = Gtk::manage(Glib::wrap(GTK_WIDGET(vte_terminal_new()), false));
		terminal->set_expand(true);
		vte = VTE_TERMINAL(terminal->gobj());

		vte_terminal_reset(vte, true, true);
		vte_terminal_set_input_enabled(vte, false);
		GdkRGBA color{0, 0, 0, 0};
		vte_terminal_set_color_cursor_foreground(vte, &color);

		entry.add_css_class("no-radius");
		entry.add_css_class("monospace");

		auto target = Gtk::DropTarget::create(G_TYPE_FILE, Gdk::DragAction::COPY);

		target->signal_drop().connect([this](const Glib::ValueBase &base, double, double) {
			if (base.gobj()->g_type != G_TYPE_FILE)
				return false;

			auto file = Glib::wrap(G_FILE(g_value_get_object(base.gobj())), true);
			std::string path = file->get_path();
			std::string javascript;

			try {
				javascript = readFile(path);
			} catch (const std::exception &err) {
				std::string message = std::format("Reading from {} failed: {}", path.c_str(), err.what());
				ERROR_(message);
				vte_terminal_feed(vte, "\e[31m", 5);
				vte_terminal_feed(vte, message.data(), message.size());
				vte_terminal_feed(vte, "\e[39m\r\n", 7);
				return false;
			}

			vte_terminal_feed(vte, "\e[2m>\e[22m [", 12);
			vte_terminal_feed(vte, path.data(), path.size());
			vte_terminal_feed(vte, "]\r\n", 3);
			runScript(javascript);
			return true;
		}, true);

		terminal->add_controller(target);

		vbox.set_expand(true);
		vbox.append(*terminal);
		vbox.append(entry);

		entry.signal_activate().connect([&] {
			if (computer) {
				Glib::ustring text = entry.get_text();
				if (text.empty())
					return;
				const std::string &script = text.raw();
				vte_terminal_feed(vte, "\e[2m>\e[22m ", 11);
				vte_terminal_feed(vte, script.data(), script.size());
				vte_terminal_feed(vte, "\r\n", 2);
				runScript(script);
				entry.set_text("");
			}
		});

		game->getWindow().addYield(entry);
		entry.signal_realize().connect([this] {
			entry.grab_focus();
		});
	}

	Gtk::Widget & ComputerModule::getWidget() {
		return vbox;
	}

	void ComputerModule::reset() {

	}

	void ComputerModule::update() {

	}

	std::optional<Buffer> ComputerModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && computer && source->getGID() == computer->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "ScriptResult" || name == "ScriptError" || name == "ScriptPrint") {

			auto *buffer = std::any_cast<Buffer>(&data);
			buffer->take<Token>();
			std::string result = buffer->take<std::string>();

			if (name == "ScriptError") {
				vte_terminal_feed(vte, "\e[31m", 5);
				vte_terminal_feed(vte, result.data(), result.size());
				vte_terminal_feed(vte, "\e[39m", 5);
			} else {
				vte_terminal_feed(vte, result.data(), result.size());
			}

			vte_terminal_feed(vte, "\r\n", 2);

		} else if (name == "GetAgentGID") {

			return Buffer{computer->getGID()};

		}

		return {};
	}

	void ComputerModule::runScript(const std::string &script) {
		game->getPlayer()->sendMessage(computer, "RunScript", ServerGame::generateRandomToken(), script);
	}
}
