#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <memory>
#include <string>
#include <vector>

namespace Game3 {
	class Agent;
	class Gene;
	class Mutator;

	class GeneInfoModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/gene_info"}; }

			GeneInfoModule(std::shared_ptr<Gene>);

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void update(std::shared_ptr<Gene>);

		private:
			std::shared_ptr<Gene> gene;
			std::vector<Gtk::Label> labels;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
	};
}
