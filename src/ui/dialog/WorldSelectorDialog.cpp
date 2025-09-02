#include "data/GameDB.h"
#include "graphics/Texture.h"
#include "ui/dialog/WorldSelectorDialog.h"
#include "ui/widget/Box.h"
#include "ui/widget/Icon.h"
#include "ui/widget/Label.h"
#include "ui/widget/Scroller.h"
#include "ui/Constants.h"
#include "ui/UIContext.h"
#include "ui/Window.h"
#include "util/Format.h"

namespace Game3 {
	namespace {
		constexpr float WIDTH = 150;
		constexpr float HEIGHT = 100;
	}

	WorldSelectorDialog::WorldSelectorDialog(UIContext &ui, float selfScale):
		LoadFileDialog(ui, selfScale, "Load World", WIDTH, HEIGHT) {}

	bool WorldSelectorDialog::filter(const std::filesystem::directory_entry &entry) const {
		return entry.path().extension() == GameDB::getFileExtension();
	}

	bool WorldSelectorDialog::isFile(const std::filesystem::directory_entry &entry) const {
		std::error_code code{};
		return (entry.is_regular_file(code) && !code) || entry.path().extension() == GameDB::getFileExtension();
	}
}
