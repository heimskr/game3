#include "game/Game.h"
#include "realm/Cave.h"
#include "realm/House.h"
#include "realm/Overworld.h"
#include "realm/RealmFactory.h"
#include "realm/ShadowRealm.h"

namespace Game3 {
	void Game::add(RealmFactory &&factory) {
		auto shared = std::make_shared<RealmFactory>(std::move(factory));
		registry<RealmFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addRealms() {
		auto &types = registry<RealmTypeRegistry>();
		auto &factories = registry<RealmFactoryRegistry>();

		auto addRealm = [&]<typename T>(const Identifier &id) {
			types.add(id);
			factories.add(id, std::make_shared<RealmFactory>(RealmFactory::create<T>(id)));
		};

		// ...
		addRealm.operator()<Overworld>(Overworld::ID());
		addRealm.operator()<ShadowRealm>(ShadowRealm::ID());
		addRealm.operator()<House>(House::ID());
		addRealm.operator()<Realm>("base:realm/blacksmith");
		addRealm.operator()<Cave>(Cave::ID());
		addRealm.operator()<Realm>("base:realm/tavern");
		addRealm.operator()<Realm>("base:realm/keep");
	}

	RealmPtr Game::tryRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		if (auto iter = realms.find(realm_id); iter != realms.end())
			return iter->second;
		return {};
	}

	RealmPtr Game::getRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		return realms.at(realm_id);
	}

	RealmPtr Game::getRealm(RealmID realm_id, const std::function<RealmPtr()> &creator) {
		auto lock = realms.uniqueLock();
		if (auto iter = realms.find(realm_id); iter != realms.end())
			return iter->second;
		RealmPtr new_realm = creator();
		realms.emplace(realm_id, new_realm);
		return new_realm;
	}

	void Game::addRealm(RealmID realm_id, RealmPtr realm) {
		auto lock = realms.uniqueLock();
		if (!realms.emplace(realm_id, realm).second)
			throw std::runtime_error("Couldn't add realm " + std::to_string(realm_id) + ": a realm with that ID already exists");
	}

	void Game::addRealm(RealmPtr realm) {
		addRealm(realm->id, realm);
	}

	bool Game::hasRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		return realms.contains(realm_id);
	}

	void Game::removeRealm(RealmID realm_id) {
		RealmPtr realm = getRealm(realm_id);
		realm->onRemove();
		auto lock = realms.uniqueLock();
		realms.erase(realm_id);
	}
}