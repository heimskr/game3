#pragma once

#include "types/Types.h"
#include "ui/module/Module.h"

#include <any>
#include <memory>
#include <string>
#include <vector>

namespace Game3 {
	class Agent;
	class Gene;
	class Mutator;

	class GeneInfoModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/gene_info"}; }

			GeneInfoModule(std::shared_ptr<ClientGame>, const std::any &);
			GeneInfoModule(std::shared_ptr<ClientGame>, std::shared_ptr<Gene>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;

			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Gene> gene;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::Label infoLabel;
	};
}
