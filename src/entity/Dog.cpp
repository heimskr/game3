#include "entity/Dog.h"
#include "game/Game.h"

namespace Game3 {
	std::vector<Identifier> Dog::breeds{
		"base:texture/dog",
		"base:texture/hellhound",
	};

	bool Dog::canAbsorbGenes(const nlohmann::json &genes) const {
		return checkGenes(genes, {"species", "breed"});
	}

	void Dog::absorbGenes(const nlohmann::json &genes) {
		absorbGene(breed, genes, "breed");
		texture = nullptr;
	}

	void Dog::iterateGenes(const std::function<void(Gene &)> &function) {
		function(species);
		function(breed);
	}

	void Dog::iterateGenes(const std::function<void(const Gene &)> &function) const {
		function(species);
		function(breed);
	}

	void Dog::render(const RendererContext &renderers) {
		if (!texture) {
			// It's assumed they're all the same variety.
			texture = getGame()->registry<TextureRegistry>().at(breeds.at(breed.getValue()));
			assert(texture);
		}

		Animal::render(renderers);
	}

	void Dog::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << breed;
	}

	void Dog::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> breed;
		texture = nullptr;
	}

	void Dog::setBreed(LongGene::ValueType new_breed) {
		breed.setValue(new_breed);
		texture = nullptr;
	}
}
