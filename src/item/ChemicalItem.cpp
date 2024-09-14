#include "tools/Flasker.h"
#include "chemistry/MoleculeColors.h"
#include "chemistry/MoleculeNames.h"
#include "graphics/HSL.h"
#include "graphics/OpenGL.h"
#include "graphics/Texture.h"
#include "item/ChemicalItem.h"

#include <random>

namespace Game3 {
	Lockable<std::unordered_map<std::string, Glib::RefPtr<Gdk::Pixbuf>>> ChemicalItem::imageCache{};
	Lockable<std::unordered_map<std::string, TexturePtr>> ChemicalItem::textureCache{};

	Glib::RefPtr<Gdk::Pixbuf> ChemicalItem::getImage(const Game &game, const ConstItemStackPtr &stack) const {
		const std::string formula = getFormula(*stack);

		{
			auto shared_lock = imageCache.sharedLock();
			if (auto iter = imageCache.find(formula); iter != imageCache.end())
				return iter->second;
		}

		auto unique_lock = imageCache.uniqueLock();
		Glib::RefPtr<Gdk::Pixbuf> image = makeImage(game, stack);
		imageCache.emplace(formula, image);
		return image;
	}

	TexturePtr ChemicalItem::getTexture(const Game &game, const ConstItemStackPtr &stack) const {
		const std::string formula = getFormula(*stack);

		{
			auto shared_lock = textureCache.sharedLock();
			if (auto iter = textureCache.find(formula); iter != textureCache.end())
				return iter->second;
		}

		auto unique_lock = textureCache.uniqueLock();
		TexturePtr image = makeTexture(game, stack);
		textureCache.emplace(formula, image);
		return image;
	}

	TexturePtr ChemicalItem::makeTexture(const Game &, const ConstItemStackPtr &stack) const {
		const std::string formula = getFormula(*stack);

		HSL hsl;

		if (!formula.empty()) {
			if (auto iter = moleculeColors.find(formula); iter != moleculeColors.end()) {
				hsl = iter->second;
			} else {
				const size_t hash = std::hash<std::string>{}(formula);
				hsl.h = hash % 360;
				hsl.s = (hash / double(std::numeric_limits<size_t>::max())) / 2.f + .5f;
				std::default_random_engine rng(hash);
				std::normal_distribution<float> normal(0, 0.1);
				hsl.l = normal(rng);
			}
		}

		int width{};
		int height{};

		rawImage = generateFlaskRaw("resources/testtubebase.png", "resources/testtubemask.png", hsl.h, hsl.s, hsl.l, &width, &height);

		TexturePtr new_texture = std::make_shared<Texture>();
		new_texture->alpha = true;
		new_texture->filter = GL_NEAREST;
		new_texture->format = GL_RGBA;
		new_texture->init(rawImage, width, height);

		return new_texture;
	}

	std::string ChemicalItem::getTooltip(const ConstItemStackPtr &stack) {
		std::string formula = getFormula(*stack);
		if (formula.empty())
			return "Unknown Chemical";
		if (auto iter = moleculeNames.find(formula); iter != moleculeNames.end())
			return iter->second + " (" + formula + ')';
		return formula;
	}

	std::string ChemicalItem::getFormula(const ItemStack &stack) {
		if (auto iter = stack.data.find("formula"); iter != stack.data.end() && !iter->is_null())
			return *iter;
		return {};
	}
}
