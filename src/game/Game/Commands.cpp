#include "game/Game.h"
#include "command/local/ChemicalCommand.h"
#include "command/local/LocalCommandFactory.h"
#include "command/local/LoginCommand.h"
#include "command/local/PlayersCommand.h"
#include "command/local/RegisterCommand.h"
#include "command/local/SegfaultCommand.h"
#include "command/local/UsageCommand.h"

namespace Game3 {
	void Game::add(LocalCommandFactory &&factory) {
		auto shared = std::make_shared<LocalCommandFactory>(std::move(factory));
		registry<LocalCommandFactoryRegistry>().add(shared->name, shared);
	}

	void Game::addLocalCommandFactories() {
		add(LocalCommandFactory::create<RegisterCommand>());
		add(LocalCommandFactory::create<LoginCommand>());
		add(LocalCommandFactory::create<UsageCommand>());
		add(LocalCommandFactory::create<ChemicalCommand>());
		add(LocalCommandFactory::create<PlayersCommand>());
		add(LocalCommandFactory::create<SegfaultCommand>());
	}
}
