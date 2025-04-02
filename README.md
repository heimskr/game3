# I can't believe I have to say this, but *do not open AI slop PRs*.

[![wakatime](https://wakatime.com/badge/github/heimskr/game3.svg)](https://wakatime.com/badge/github/heimskr/game3)

# Game3
A 2D game made with OpenGL. No engine because I'd prefer to do things my own way.

## Screenshots

See [the game3 website](https://game3.gay/) for screenshots and more information.

## Credits
Huge thanks to [Shade](https://merchant-shade.itch.io/) for creating [Mini World Sprites](https://merchant-shade.itch.io/16x16-mini-world-sprites),
which features heavily in this project, and for creating custom anvil and furnace tiles at my request.

Thanks to Tilation for their [indoor tileset](https://tilation.itch.io/16x16-small-indoor-tileset).

Thanks to [Vadim](https://github.com/Vadimuh), [Fayabella](https://github.com/Fayabella) and [Bamboozle.jpg](https://github.com/Bamboozle-jpg) for their flower sprites.

Thanks to Fayabella again for her work on many other sprites and for her many ideas and feedback over the course of the game's development.

Thanks to [190n](https://github.com/190n) for their work on the resource embedding system (since removed).

Sprites credited to [Franuka](https://twitter.com/franuka_art) are not governed by this project's license.

## Contribution Ideas
If you'd like to contribute, here are some ideas, ordered roughly from easiest to hardest.

- Don't use AI. At all. It's not that hard. Slop PRs will be reported as spam.
- Improve the shading of the flower sprites `flower1_*` and `flower5_*` in `/resources/tileset`.
- Make animals useful.
	- For example, add a shears tool that can be used to get wool from sheep.
- Add a plugin system similar to that of [Algiz](https://github.com/heimskr/algiz)
	(see [PluginHost.cpp](https://github.com/heimskr/algiz/blob/master/src/PluginHost.cpp),
	[PluginHost.h](https://github.com/heimskr/algiz/blob/master/include/plugins/PluginHost.h),
	[Plugin.h](https://github.com/heimskr/algiz/blob/master/include/plugins/Plugin.h)) that makes use of the game's registry system
	(see Game::initRegistries through Game::initialSetup in [Game.cpp](https://github.com/heimskr/game3/blob/master/src/game/Game.cpp)).
	- You will need to implement dependency checking to make sure plugins are loaded in the right order and that there are no cycles;
		[Graph::topoSort](https://github.com/heimskr/game3/blob/master/include/graph/Graph.h) may help with that.
	- Bonus points if you can get interop with other languages (e.g., Rust or Zig) working. This will likely be difficult because plugins will typically need to extend classes like Entity, TileEntity or Realm that have virtual methods.
- Rewrite the "engine" to be based on Vulkan or SDL2 instead of OpenGL.
