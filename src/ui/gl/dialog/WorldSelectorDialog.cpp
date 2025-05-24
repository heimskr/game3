#include "data/GameDB.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/WorldSelectorDialog.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"
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
}
