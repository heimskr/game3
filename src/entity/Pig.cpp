#include "entity/Pig.h"
#include "game/Game.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/Color.h"
#include "graphics/Recolor.h"
#include "graphics/RendererContext.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	std::vector<Identifier> Pig::variants{
		"base:texture/pig_pink",
		"base:texture/pig_black",
		"base:texture/pig_white",
	};

	Pig::Pig():
		Entity(ID()),
		Animal(),
		species("species", ID().str()),
		variant("variant", 0, 2, 0) {}

	std::vector<ItemStackPtr> Pig::getDrops() {
		std::vector<ItemStackPtr> out = Animal::getDrops();
		out.push_back(ItemStack::create(getGame(), "base:item/raw_meat"));
		return out;
	}

	bool Pig::canAbsorbGenes(const nlohmann::json &genes) const {
		return checkGenes(genes, {"variant", "species"});
	}

	void Pig::absorbGenes(const nlohmann::json &genes) {
		absorbGene(variant, genes, "variant");
		texture = nullptr;
	}

	void Pig::iterateGenes(const std::function<void(Gene &)> &function) {
		function(variant);
		function(species);
	}

	void Pig::iterateGenes(const std::function<void(const Gene &)> &function) const {
		function(variant);
		function(species);
	}

	void Pig::render(const RendererContext &renderers) {
		if (!texture) {
			// It's assumed they're all the same variety.
			texture = getGame()->registry<TextureRegistry>().at(variants.at(variant.getValue()));
			assert(texture);
		}

		Animal::render(renderers);
	}

	void Pig::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << variant;
	}

	void Pig::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> variant;
		texture = nullptr;
	}
}
