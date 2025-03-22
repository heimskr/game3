#include "statuseffect/StatusEffect.h"

namespace Game3 {
	StatusEffect::StatusEffect(Identifier identifier):
		NamedRegisterable(std::move(identifier)) {}

	StatusEffect::~StatusEffect() = default;

	void StatusEffect::replenish(const std::shared_ptr<LivingEntity> &) {}

	void StatusEffect::onAdd(const std::shared_ptr<LivingEntity> &) {}

	void StatusEffect::onRemove(const std::shared_ptr<LivingEntity> &) {}

	void StatusEffect::modifyColors(Color &, Color &) {}

	std::shared_ptr<Texture> StatusEffect::getTexture(const std::shared_ptr<ClientGame> &) {
		return {};
	}
}
