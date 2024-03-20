#include "entity/Crab.h"
#include "game/Game.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	std::map<LongGene::ValueType, Identifier> Crab::breeds{
		{0, "base:texture/crab"},
		{8, "base:texture/crab_blue"},
	};

	bool Crab::canAbsorbGenes(const nlohmann::json &genes) const {
		return checkGenes(genes, {"species", "breed"});
	}

	void Crab::absorbGenes(const nlohmann::json &genes) {
		absorbGene(breed, genes, "breed");
		texture = nullptr;
	}

	void Crab::iterateGenes(const std::function<void(Gene &)> &function) {
		function(species);
		function(breed);
	}

	void Crab::iterateGenes(const std::function<void(const Gene &)> &function) const {
		function(species);
		function(breed);
	}

	void Crab::render(const RendererContext &renderers) {
		if (!texture) {
			// It's assumed they're all the same variety.
			Identifier texture_id;

			if (auto iter = breeds.find(breed.getValue()); iter != breeds.end()) {
				texture_id = iter->second;
			} else {
				texture_id = breeds.at(0);
			}

			texture = getGame()->registry<TextureRegistry>().at(texture_id);
			assert(texture);
		}

		Animal::render(renderers);
	}

	void Crab::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << breed;
	}

	void Crab::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> breed;
		texture = nullptr;
	}

	void Crab::setBreed(LongGene::ValueType new_breed) {
		breed.setValue(new_breed);
		texture = nullptr;
	}

	LongGene::ValueType Crab::sampleBreed() {
		return threadContext.random(0, 10);
	}
}
