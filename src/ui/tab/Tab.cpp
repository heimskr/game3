#include "ui/tab/Tab.h"

namespace Game3 {
	void Tab::hide() {
		notebook.remove_page(getWidget());
	}

	void Tab::show() {
		auto &widget = getWidget();
		if (notebook.get_page(widget))
			return;
		notebook.append_page(widget);
	}
}
