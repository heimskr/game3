# Packets

Packets are encoded as a little-endian 2-byte integer representing the packet type, followed by a little-endian 4-byte integer representing the payload length.

## Packet Types

0. Not used.

1. **Protocol Version**: informs the other side of the protocol version in use. Currently unused.

	- `u32` Version

2. **Tile Entity**: informs a client of a tile entity's data.

	- `u64` Global ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Tile entity data

3. **Chunk Request**: asks the server to send Chunk Tiles packets for a given list of chunks.

	- `i32` Realm ID
	- `bool` Generate missing chunks
	- `list<u32, 4n>` Chunk positions

	The chunk positions will be sent as sequential `(u32(x), u32(y), low32(threshold), high32(threshold))` tuples. Probably best to see the implementation in src/packets/ChunkRequestPacket.cpp.

4. **Tile Update**: informs the client of the new tile ID for a single tile.

	- `i32` Realm ID
	- `u8` Layer
	- `i64` Position Y
	- `i64` Position X
	- `u16` Tile ID

5. **Command Result**: informs a client of the result of a command.

	- `u64` Command ID
	- `bool` Success
	- `string` Message

6. **Command**: sent to the server to run a command.

	- `u64` Packet GID
	- `string` Command

7. **Self Teleported**: tells a client the position of their player.

	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X

8. **Chunk Tiles**: sends all the terrain data for a chunk to a client.

	- `i32` Realm ID
	- `i32` Chunk position X
	- `i32` Chunk position Y
	- `u64` Update counter
	- `list<u16, 4*64**2>` Tile IDs (layer 1, then 2, then 3, then 4)
	- `list<u64, 64**2>` Fluid Tiles

	Note that the entire payload above is compressed with lz4. The compressed data is sent as a `list<u8>`.

9. **Realm Notice**: informs a client of the existence of a realm.

	- `i32` Realm ID
	- `string` Realm type
	- `string` Tileset identifier
	- `bool` Outdoors

10. **Login**: sent by a client to log into the server.

	- `string` Username
	- `u64` Token
	- `optional<string>` Display name (for automatic registration in case the token is the omnitoken and the user doesn't exist)

11. **Login Status**: informs a client of the result of a login attempt.

	- `bool` Success
	- `u64` Global ID
	- `string` Username
	- `string` Display name
	- `string` Message (in case of failure)
	- `...` Player data

	Player data sent only if successful.

12. **Register Player**: sent by a client to register a new player account.

	- `string` Username
	- `string` Display name

13. **Registration Status**: informs a client of the status of their registration attempt.

	- `string` Username
	- `string` Display name
	- `u64` Token

	The token will be 0 if the attempt failed, or an arbitrary nonzero value otherwise.

14. **Entity**: informs a client of an entity's data.

	- `u64` Global ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Entity data

	The Entity Data varies with entity type, but it always begins with:

	- `string` Entity type
	- `u64` Global ID
	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X
	- `u8` Facing direction
	- `u64` Update counter
	- `f32` Offset X
	- `f32` Offset Y
	- `f32` Offset Z
	- `f32` Velocity X
	- `f32` Velocity Y
	- `f32` Velocity Z
	- `list<u8>` Path
	- `u64` Money

15. **Move Player**: tells the server to move the player to a given location.

	- `i64` New position Y
	- `i64` New position X
	- `u8` Movement direction
	- `optional<u8>` Facing direction

16. **Error**: informs a client of an error.

	- `string` Error

17. **Entity Moved**: informs a client that an entity moved.

	- `u64` Entity global ID
	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X
	- `u8` Facing
	- `optional<shortlist<f32, 3>>` New offset
	- `optional<f32>` New z-speed
	- `bool` Adjust offset
	- `bool` Is teleport

	The adjust offset flag, if true, tells the client to set the offset so that the entity's visible position
	(i.e., the integer coordinates + the float offsets) appears unchanged after the integer coordinates are updated.

18. **Send Chat Message**: tells the server to send a chat message.

	- `string` Message

19. **Entity Set Path**: informs a client that an entity chose a new path.

	- `u64` Entity global ID
	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X
	- `list<u8>` Path directions
	- `u64` New update counter

20. **Teleport Self**: tells the server to teleport the player.

	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X

21. **Interact**: tells the server or a client to perform an interaction.

	- `bool` Direct: whether to interact with the tile the player is on, rather than one tile in front of the player
	- `u8` Use item: which hand's held item to use (e.g., for attacking) (0 = neither, 1 = left, 2 = right)
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)
	- `optional<u64>` Global ID of specific agent to interact with
	- `optional<u8>` Facing direction

22. **Inventory Slot Update**: tells a client to set an inventory slot.

	- `i32` Slot
	- `ItemStack`

23. **Destroy Entity**: tells a client to remove an entity.

	- `u64` Global ID
	- `optional<i32>` Realm requirement

	If the realm requirement is set, the entity will not be destroyed unless its current realm ID (as far as the client knows) matches the realm requirement.

24. **Inventory**: sends a client its player's inventory.

	- `Inventory` Inventory

25. **Set Active Slot**: tells the server or a client to set the active slot in the player's inventory.

	- `i32` Slot

26. **Open Item Filters**: tells a client to open the item filters module.

	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X
	- `u8` Direction
	- `bool` Remove on move

27. **Destroy Tile Entity**: tells a client to remove a tile entity.

	- `u64` Global ID

28. **Entity Forgotten**: informs the server that the client has removed an entity from its memory.

	- `u64` Global ID
	- `i32` Realm ID

	Currently unused.

29. **Click**: informs the server that the client clicked on a given square.

	- `i64` Position Y
	- `i64` Position X
	- `f32` Offset X
	- `f32` Offset Y
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)

30. **Time**: informs a client of the game time.

	- `f64` Time

31. **Craft**: tells the server to craft something.

	- `u64` Packet GID
	- `u64` Recipe index
	- `u64` Count

32. **Continuous Interaction**: tells the server to start or stop continuous interaction.

	- `optional<u8>` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)

	Continuous interaction starts if Modifiers is present, stops if not.

33. **Fluid Update**: informs a client of the new fluid state for a single tile.

	- `i32` Realm ID
	- `i64` Position Y
	- `i64` Position X
	- `u64` Fluid ID, level and infiniteness

	The lower 16 bits of the `u64` are the fluid ID, the next 16 bits are the fluid level and the bit after that indicates whether the fluid tile is infinite.

34. **Held Item Set**: tells a client that an entity's held item changed.

	- `i32` Realm ID
	- `u64` Entity global ID
	- `bool` Hand
	- `i32` Slot
	- `u64` New update counter

	Hand is `true` for left, `false` for right.

	A negative Slot value indicates that the entity is no longer holding an item in the specified hand.

35. **Set Held Item**: tells the server to set the player's held item.

	- `bool` Hand
	- `i32` Slot

	Hand is `true` for left, `false` for right.

	A negative Slot value indicates that the player should no longer be holding an item in the specified hand.

36. **Entity Request**: asks the server to send entities' data if the client versions are stale.

	- `i32` Realm ID
	- `list<u64, 2n>` Global ID + threshold pairs

37. **Tile Entity Request**: asks the server to send tile entities' data if the client versions are stale.

	- `i32` Realm ID
	- `list<u64, 2n>` Global ID + threshold pairs

38. **Jump**: tells the server to make the player jump. No payload.

39. **Drop Item**: tells the server to drop or discard an item from the player's inventory.

	- `i32` Slot
	- `bool` Discard

40. **Open Module For Agent**: tells a client to open a module for a given agent.

	- `string` Module identifier
	- `u64` Agent global ID
	- `bool` Remove on player movement

41. **Swap Slots**: tells the server to swap a slot in one agent's inventory with a slot in another agent's inventory.

	- `u64` First agent global ID
	- `u64` Second agent global ID
	- `i32` First slot
	- `i32` Second slot
	- `u16` First inventory index (usually 0)
	- `u16` Second inventory index (usually 0)

42. **Move Slots**: tells the server to move a slot in one agent's inventory into a slot in another agent's inventory.

	- `u64` First agent global ID
	- `u64` Second agent global ID
	- `i32` First slot
	- `i32` Second slot
	- `u16` First inventory index (usually 0)
	- `u16` Second inventory index (usually 0)

	This differs from Swap Slots in that compatible stacks will be merged instead of swapped
	and that the first slot has to have something in it.

43. **Agent Message**: tells the server to send a message from the player to an agent, or tells a client to send a message from an agent to the player.

	- `u64` Agent global ID
	- `string` Message name
	- `...` Message data

44. **Set Tile Entity Energy**: tells a client that a tile entity's stored energy has changed.

	- `u64` Tile entity global ID
	- `u64` Energy amount

45. **Set Player Station Types**: tells a client what their available crafting station types are.

	- `list<string>` Identifiers

46. **Entity Changing Realms**: tells a client that an entity is about to teleport to a different realm.

	- `u64` Entity global ID
	- `i32` New realm ID
	- `i64` New position Y
	- `i64` New position X

47. **Chat Message Sent**: tells a client that a player sent a chat message.

	- `u64` Player global ID
	- `string` Message

48. **Display Text**: tells a client to add a text module and display a message.

	- `string` Name to display
	- `string` Message
	- `bool` Remove on move: Whether to close the tab when the player moves

49. **Drag**: tells the server that the client dragged the mouse on a given square.

	- `u8` Action (1 => drag started, 2 => drag updated, 3 => drag ended)
	- `i64` Position Y
	- `i64` Position X

50. **Set Item Filters**: tells the server to set the item filters for an edge of a pipe.

	- `u64` Pipe global ID
	- `u8` Direction
	- `ItemFilter` Item filter data

51. **Tileset Request**: asks the server for the tileset data.

	- `i32` Realm ID

52. **Tileset Response**: gives the client a map of numeric tile IDs to string tile IDs.

	- `i32` Realm ID
	- `map<u16, string>` Tileset data

53. **Recipe List**: gives the client a list of the recipes the server knows.

	- `string` Recipe type identifier
	- `list<string>` Recipe JSONs

	The order of the list is important because the client and server have to agree on the order of the recipes.

54. **Living Entity Health Changed**: informs the client of a living entity's new health.

	- `u64` Entity global ID
	- `i32` New health

55. **Use Item**: tells the server to use an item.

	- `i32` Slot
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)

56. **Subscribe to Village Updates**: tells the server to start/stop sending village updates to the client.

	- `optional<u64>` Village ID: if empty, unsubscribe

57. **Village Update**: informs the client of a village's latest data.

	- `u64` Village ID
	- `i32` Realm ID
	- `i32` Chunk position X
	- `i32` Chunk position Y
	- `i64` Position Y
	- `i64` Position X
	- `string` Village name
	- `f64` Available labor
	- `f64` Greed factor
	- `map<string, f64>` Resources

58. **Open Village Trade**: tells a client to open the village trade module.

	- `u64` Village ID
	- `bool` Remove on move

59. **Do Village Trade**: tells the server to perform a village trade.

	- `u64` Village ID
	- `string` Resource
	- `u64` Amount
	- `bool` Is sell (true = sell, false = buy)

60. **Entity Money Changed**: tells a client that an entity's owned money changed.

	- `u64` Global ID
	- `u64` Money count

61. **Entity Ridden**: tells a client that one entity has started riding another.

	- `optional<u64>` Riding entity's global ID
	- `u64` Ridden entity's global ID

62. **Set Copier Configuration**: tells the server to change the configuration of a copier tool in the player's inventory.

	- `i32` Slot
	- `bool` Include tile entities

63. **Play Sound**: tells a client to play a given sound at a given position.

	- `string` Sound ID
	- `Position` Origin
	- `f32` Pitch
	- `u16` Maximum distance (65535 = audible everywhere)

64. **Known Items**: tells a client which items it has ever seen.

	- `list<string>` Item IDs

65. **Add Known Item**: tells a client to add an item to its known items set.

	- `string` Item ID

66. **Submit Score**: informs the server of the score the player got while playing a minigame.

	- `string` Minigame ID
	- `u64` Score

	There's no anti-cheat of any kind so you could totally just submit a score of 18 quintillion and get away with it.
	We're trusting you though. DFIU. Sei treu und gedeihe.

67. **Open Minigame**: tells a client to open a minigame.

	- `string` Minigame ID
	- `i32` Viewport width
	- `i32` Viewport height

68. **Set Firing**: tells the server that the client is or isn't holding down the mouse on the world.

	- `bool` Firing

69. **Use Fluid Gun**: tells the server to activate the client's held fluid gun.

	- `i64` Position Y
	- `i64` Position X
	- `f32` Offset X
	- `f32` Offset Y
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)
	- `u16` Tick frequency

70. **Status Effects**: tells a client what status effects a living entity currently has.

	- `u64` Global ID
	- `StatusEffectMap`

71. **Update Agent Field**: tells a client to update a field of an agent.

	- `u64` Global ID
	- `u32` Field name hash (32-bit FNV1A; see `/util/ConstexprHash.h`).
	- `...` Field value

72. **Explosion**: tells a client to cause the client-side effects of an explosion.

	- `i32` Realm ID
	- `Position` Origin
	- `f32` Radius
	- `string` Particle type
	- `optional<string>` Sound effect ID
	- `optional<f32>` Sound effect pitch variance
	- `...` Particle randomization parameters

# Message Format

All values are little endian. Strings are not null-terminated.

## Types

| Type Encoding                    | Type                         |
|:---------------------------------|:-----------------------------|
| `0x00`                           | Unused                       |
| `0x01`                           | `u8`/`bool`                  |
| `0x02`                           | `u16`                        |
| `0x03`                           | `u32`                        |
| `0x04`                           | `u64`                        |
| `0x05`                           | `i8`                         |
| `0x06`                           | `i16`                        |
| `0x07`                           | `i32`                        |
| `0x08`                           | `i64`                        |
| `0x09`                           | `f32`                        |
| `0x0a`                           | `f64`                        |
| `0x0b` . type                    | optional&lt;type&gt;         |
| `0x0c`                           | optional (empty)             |
| [`0x10`, `0x1f`)                 | string of length [0, 15)     |
| `0x1f`                           | string of arbitrary length   |
| `0x20` . type                    | list&lt;type&gt;             |
| `0x21` . type[key] . type[value] | map&lt;key, value&gt;        |
| [`0x30`, `0x3f`] . type          | shortlist of length [1, 16]  |
| `0xe0`                           | ItemStack                    |
| `0xe1`                           | Inventory                    |
| `0xe2`                           | FluidStack                   |
| `0xe3`                           | ItemFilter                   |
| `0xe4`                           | ItemFilter::Config           |
| `0xe5`                           | FloatGene                    |
| `0xe6`                           | LongGene                     |
| `0xe7`                           | CircularGene                 |
| `0xe8`                           | StringGene                   |
| `0xe9`                           | Position                     |
| `0xea`                           | StatusEffectMap              |

Note that string types are always encoded as `0x1f` when used as a subtype of a list or a map, and optional types are always encoded as `0x0b` followed by the subtype in the same scenario.

## Values

To send a numeric type (`0x01` through `0x0a`), send its type encoding followed by its little-endian representation. For example, the unsigned 16-bit integer `0x1234` would be sent as `0x02 0x34 0x12`.

To send an optional value, send `0x0c` if it's empty, or `0x0b` followed by the type and value. For example, an optional signed 8-bit integer with a value of `0x64` would be sent as `0x0b 0x01 0x64`, whereas if it lacked a value it would be sent as `0x0c`.
<!-- TODO: do empty optionals also require the type to be appended? -->
<!-- For now, no. -->

To send an empty string, send `0x10`.

To send a string of length 1 through 14 (inclusive), send `0x11` through `0x1d` followed by the string.

To send a string of more than 14 characters, send `0x1f`, followed by the string length as an unsigned little-endian 32-bit integer, followed by the string. For example, the string "Example String" would be sent as `0x1f 0x11 0x00 0x00 0x00 0x45 0x78 0x61 0x6d 0x70 0x6c 0x65 0x20 0x53 0x65 0x6e 0x74 0x65 0x6e 0x63 0x65`.

To send a list, send `0x20`, followed by the type encoding of the subtype, followed by the number of items in the list as an unsigned little-endian 32-bit integer, followed by the list of values without type encodings (except for variable-length types such as optionals, strings, lists and maps, which must include their full type encodings before each key or value). For example, the list of unsigned 8-bit integers 1, 2, 3, 4, 5 would be sent as `0x20 0x01 0x05 0x00 0x00 0x00 0x01 0x02 0x03 0x04 0x05`.

To send a map, send `0x21`, followed by the type encoding of the key type, followed by the type encoding of the value type, followed by the number of key-value pairs as an unsigned little-endian 32-bit integer, followed by the key-value pairs without type encodings (except for variable-length types such as optionals, strings, lists and maps, which must include their full type encodings before each key or value). For example, the map of unsigned 8-bit integers to strings `{0x30 => "", 0x40 => "Hi", 0x50 => "This is a long string"}` would be sent as `0x21 0x01 0x1f 0x03 0x00 0x00 0x00 0x30 0x10 0x40 0x12 0x48 0x69 0x50 0x1f 0x15 0x00 0x00 0x00 0x54 0x68 0x69 0x73 0x20 0x69 0x73 0x20 0x61 0x20 0x6c 0x6f 0x6e 0x67 0x20 0x73 0x74 0x72 0x69 0x6e 0x67`.

### ItemStack (`0xe0`)

- `string` Item ID
- `u64` Item Count
- `string` Item Extra Data (JSON)

### Inventory (`0xe1`)

- `u64` Inventory Owner
- `i32` Inventory Slot Count
- `i32` Inventory Active Slot
- `map<i32, ItemStack>` Inventory Items

### FluidStack (`0xe2`)

- `u16` Fluid ID
- `u64` Fluid Amount

### ItemFilter (`0xe3`)

- `bool` Whitelist mode (false = blacklist mode)
- `bool` Strict mode
- `list<string>` Item identifiers
- `map<string, list<ItemFilter::Config>>` Configs by item

### ItemFilter::Config (`0xe4`)

- `string` Item data JSON
- `u8` Comparator (none = 0, less = 1, greater = 2)
- `u64` Item count (for comparisons)

### FloatGene (`0xe5`)

- `f32` Minimum value
- `f32` Maximum value
- `f32` Current value

### LongGene (`0xe6`)

- `i64` Minimum value
- `i64` Maximum value
- `i64` Current value

### CircularGene (`0xe7`)

- `f32` Current value

### StringGene (`0xe8`)

- `string` Current value

### Position (`0xe9`)

TODO

### StatusEffectMap (`0xea`)

TODO

# Examples

To send a chunk request in realm 42 for chunks (-1, -2), (0, 0) and (40, 64), the encoded packet would be:
- `0x03 0x00`: packet type
- `0x23 0x00 0x00 0x00`: payload length (35 bytes)
- `0x03`: type indicator (`i32`)
- `0x2a 0x00 0x00 0x00`: realm ID (42)
- `0x20 0x07`: type indicator (`list<i32>`)
- `0x03 0x00 0x00 0x00`: list length (3)
- `0xff 0xff 0xff 0xff`: x-coordinate of first chunk position (-1)
- `0xfe 0xff 0xff 0xff`: y-coordinate of first chunk position (-2)
- `0x00 0x00 0x00 0x00`: x-coordinate of second chunk position (0)
- `0x00 0x00 0x00 0x00`: y-coordinate of second chunk position (0)
- `0x28 0x00 0x00 0x00`: x-coordinate of third chunk position (40)
- `0x40 0x00 0x00 0x00`: y-coordinate of third chunk position (64)

Concatenated: `03 00 23 00 00 00 03 2a 00 00 00 20 07 03 00 00 00 ff ff ff ff fe ff ff ff 00 00 00 00 00 00 00 00 2a 00 00 00 40 00 00 00`.

### FluidTile

Fluid tiles are represented with 64-bit integers. The lower 16 bits are the fluid ID, the next 16 bits are the
fluid level and the 8 bits after that are a boolean indicating whether the fluid tile is an infinite source.
