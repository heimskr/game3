#include "graphics/Texture.h"
#include "ui/gl/dialog/WorldsDialog.h"
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
		constexpr float WIDTH = 100;
		constexpr float HEIGHT = 100;
	}

	WorldsDialog::WorldsDialog(UIContext &ui, float selfScale):
		FileChooserDialog(ui, selfScale, "Worlds", WIDTH, HEIGHT) {}

	bool WorldsDialog::filter(const std::filesystem::directory_entry &entry) const {
		return entry.path().extension() == ".db";
	}
}
