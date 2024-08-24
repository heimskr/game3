#pragma once

#include "types/Types.h"
#include "ui/module/GTKModule.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class GeneticAnalysisModule: public GTKModule {
		public:
			static Identifier ID() { return {"base", "module/genetic_analysis"}; }

			GeneticAnalysisModule(ClientGamePtr, const std::any &);
			GeneticAnalysisModule();

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset() final;
			void update() final;
			void update(const ItemStackPtr &);

		private:
			ClientGamePtr game;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			std::vector<std::unique_ptr<Gtk::Label>> labels;
			std::vector<Gtk::Separator> separators;

			void populate();
			void ponderOrb(const ItemStackPtr &);
			void analyzeGene(const ItemStackPtr &);
			void analyzeTemplate(const ItemStackPtr &);
			void addLabel(const std::string &);
			void clearText();
	};
}
