#include "biology/Gene.h"
#include "net/Buffer.h"
#include "packet/AgentMessagePacket.h"
#include "ui/gtk/Util.h"
#include "ui/module/GeneInfoModule.h"

namespace Game3 {
	GeneInfoModule::GeneInfoModule(GenePtr gene_): gene(std::move(gene_)) {
		vbox.set_hexpand(true);
		reset();
	}

	Gtk::Widget & GeneInfoModule::getWidget() {
		return vbox;
	}

	void GeneInfoModule::reset() {
		removeChildren(vbox);
		labels.clear();

		if (!gene)
			return;

		for (const std::string &line: gene->describeLong()) {
			labels.emplace_back(line);
			Gtk::Label &label = labels.back();
			label.set_xalign(0);
			label.set_margin_top(5);
			label.set_margin_start(5);
			vbox.append(label);
		}
	}

	void GeneInfoModule::update() {
		reset();
	}

	void GeneInfoModule::update(std::shared_ptr<Gene> new_gene) {
		gene = std::move(new_gene);
		reset();
	}
}
