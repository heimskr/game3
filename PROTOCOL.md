# Packets

Packets are encoded as a little-endian 2-byte integer representing the packet type, followed by a little-endian 4-byte integer representing the payload length.

## Packet Types

0. Not used.

1. **Protocol Version**: informs the other side of the protocol version in use.

	- `u32` Version

2. **Tile Entity**: informs a client of a tile entity's data.

	- `u64` Global ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Tile Entity Data

3. **Chunk Request**: asks the server to send Chunk Tiles packets for a given list of chunks.

	- `i32` Realm ID
	- `u32[4n]` Chunk Positions

	The chunk positions will be sent as sequential `(u32(x), u32(y), low32(threshold), high32(threshold))` tuples. Probably best to see the implementation in src/packets/ChunkRequestPacket.cpp.

4. **Tile Update**: informs the client of the new tile ID for a single tile.

	- `i32` Realm ID
	- `u8` Layer
	- `{i64,i64}` Position
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
	- `{i64,i64}` Position

8. **Chunk Tiles**: sends all the terrain data for a chunk to a client.

	- `i32` Realm ID
	- `{i32,i32}` Chunk Position
	- `u64` Update Counter
	- `i16[3*64**2]` Tile IDs (layer 1, then 2, then 3)

	<!-- TODO: compression -->

9. **Realm Notice**: informs a client of the existence of a realm.

	- `i32` Realm ID
	- `string` Realm Type
	- `string` Tileset Identifier
	- `bool` Outdoors

10. **Login**: sent by a client to log into the server.

	- `string` Username
	- `u64` Token

11. **Login Status**: informs a client of the result of a login attempt.

	- `bool` Success
	- `u64` Global ID
	- `string` Username
	- `string` Display Name
	- `...` Player Data

	Player data sent only if successful.

12. **Register Player**: sent by a client to register a new player account.

	- `string` Username
	- `string` Display Name

13. **Registration Status**: informs a client of the status of their registration attempt.

	- `string` Username
	- `string` Display Name
	- `u64` Token

	The token will be 0 if the attempt failed, or an arbitrary nonzero value otherwise.

14. **Entity**: informs a client of an entity's data.

	- `u64` Global ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Tile Entity Data

15. **Start Player Movement**: tells the server to start moving the player in a given direction.

	- `u8` Direction

16. **Error**: informs a client of an error.

	- `string` Error

17. **Entity Moved**: informs a client that an entity moved.

	- `u64` Entity Global ID
	- `i32` Realm ID
	- `{i64,i64}` Position
	- `u8` Facing
	- `optional<{float, float, float}>` New Offset
	- `optional<float>` New Z-Speed

18. **Stop Player Movement**: tells the server to stop moving the player in a given direction or entirely.

	- `optional<u8>` Direction

19. **Entity Set Path**: informs a client that an entity chose a new path.

	- `u64` Entity Global ID
	- `i32` Realm ID
	- `{i64,i64}` Position
	- `list<u8>` Path Directions
	- `u64` New Update Counter

20. **Teleport Self**: tells the server to teleport the player.

	- `i32` Realm ID
	- `{i64,i64}` Position

21. **Interact**: tells the server to perform an interaction.

	- `bool` Direct: whether to interact with the tile the player is on, rather than one tile in front of the player
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)

22. **Inventory Slot Update**: tells a client to set an inventory slot.

	- `i32` Slot
	- `ItemStack` Item Stack

23. **Destroy Entity**: tells a client to remove an entity.

	- `u64` Global ID

24. **Inventory**: sends a client its player's inventory.

	- `Inventory` Inventory

25. **Set Active Slot**: tells the server to set the active slot in the player's inventory.

	- `i32` Slot

26. **Active Slot Set**: tells a client that the player's active inventory slot changed.

	- `i32` Slot

27. **Destroy Tile Entity**: tells a client to remove a tile entity.

	- `u64` Global ID

28. **Entity Forgotten**: informs the server that the client has removed an entity from its memory.

	- `u64` Global ID
	- `i32` Realm ID

	Currently unused.

29. **Click**: informs the server that the client clicked on a given square.

	- `{i64,i64}` Position
	- `{float,float}` Offset
	- `u8` Modifiers: bitfield (1 = shift, 2 = ctrl, 4 = alt, 8 = super)

30. **Time**: informs a client of the game time.

	- `double` Time

31. **Craft**: tells the server to craft something.

	- `u64` Packet GID
	- `u64` Recipe Index
	- `u64` Count

32. **Continuous Interaction**: tells the server to start or stop continuous interaction.

	- `optional<u8>` Modifiers

	Continuous interaction starts if Modifiers is present, stops if not.

33. **Fluid Update**: informs a client of the new fluid state for a single tile.

	- `i32` Realm ID
	- `{i64,i64}` Position
	- `u32` Fluid ID and Level

34. **Held Item Set**: tells a client that an entity's held item changed.

	- `i32` Realm ID
	- `u64` Entity Global ID
	- `bool` Hand
	- `i32` Slot
	- `u64` New Update Counter

	Hand is `true` for left, `false` for right.

	A negative Slot value indicates that the entity is no longer holding an item in the specified hand.

35. **Set Held Item**: tells the server to set the player's held item.

	- `bool` Hand
	- `i32` Slot

	Hand is `true` for left, `false` for right.

	A negative Slot value indicates that the player should no longer be holding an item in the specified hand.

36. **Entity Request**: asks the server to send entities' data if the client versions are stale.

	- `i32` Realm ID
	- `u64[2n]` Global ID + Threshold Pairs

37. **Tile Entity Request**: asks the server to send tile entities' data if the client versions are stale.

	- `i32` Realm ID
	- `u64[2n]` Global ID + Threshold Pairs

38. **Jump**: tells the server to make the player jump. No payload.

39. **Drop Item**: tells the server to drop or discard an item from the player's inventory.

	- `i32` Slot
	- `bool` Discard

40. **Open Agent Inventory**: tells a client to open an agent's inventory.

	- `u64` Global ID
	- `string` Name

41. **Swap Slots**: tells the server to swap a slot in one agent's inventory with a slot in another agent's inventory.

	- `u64` First Agent Global ID
	- `u64` Second Agent Global ID
	- `i32` First Slot
	- `i32` Second Slot

41. **Move Slots**: tells the server to move a slot in one agent's inventory into a slot in another agent's inventory.

	- `u64` First Agent Global ID
	- `u64` Second Agent Global ID
	- `i32` First Slot
	- `i32` Second Slot

	This differs from Swap Slots in that compatible stacks will be merged instead of swapped
	and that the first slot has to have something in it.

# Message Format

All values are little endian.

## Types

| Type Encoding                    | Type                       |
|:---------------------------------|:---------------------------|
| `0x00`                           | Unused                     |
| `0x01`                           | `u8`/`bool`                |
| `0x02`                           | `u16`                      |
| `0x03`                           | `u32`                      |
| `0x04`                           | `u64`                      |
| `0x05`                           | `i8`                       |
| `0x06`                           | `i16`                      |
| `0x07`                           | `i32`                      |
| `0x08`                           | `i64`                      |
| `0x09`                           | `float`                    |
| `0x0a`                           | `double`                   |
| `0x0b` . type                    | optional&lt;type&gt;       |
| `0x0c`                           | optional (empty)           |
| [`0x10`, `0x1f`)                 | string of length [0, 15)   |
| `0x1f`                           | string of arbitrary length |
| `0x20` . type                    | list&lt;type&gt;           |
| `0x21` . type[key] . type[value] | map&lt;key, value&gt;      |
| `0xe0`                           | ItemStack                  |
| `0xe1`                           | Inventory                  |

Note that string types are always encoded as `0x1f` when used as a subtype of a list or a map, and optional types are always encoded as `0x0b` followed by the subtype in the same scenario.

## Values

To send a numeric type (`0x01` through `0x0a`), send its type encoding followed by its little-endian representation. For example, the unsigned 16-bit integer `0x1234` would be sent as `0x02 0x34 0x12`.

To send an optional value, send `0x0c` if it's empty, or `0x0c` followed by the type and value. For example, an optional signed 8-bit integer with a value of `0x64` would be sent as `0x0c 0x01 0x64`, whereas if it lacked a value it would be sent as `0x0c`.
<!-- TODO: do empty optionals also require the type to be appended? -->

To send an empty string, send `0x10`.

To send a string of length 1 through 14 (inclusive), send `0x11` through `0x1d` followed by the string.

To send a string of more than 14 characters, send `0x1f`, followed by the string length as an unsigned little-endian 32-bit integer, followed by the string. For example, the string "Example String" would be sent as `0x1f 0x11 0x00 0x00 0x00 0x45 0x78 0x61 0x6d 0x70 0x6c 0x65 0x20 0x53 0x65 0x6e 0x74 0x65 0x6e 0x63 0x65`.

To send a list, send `0x20`, followed by the type encoding of the subtype, followed by the number of items in the list as an unsigned little-endian 32-bit integer, followed by the list of values without type encodings (except for variable-length types such as optionals, strings, lists and maps, which must include their full type encodings before each key or value). For example, the list of unsigned 8-bit integers 1, 2, 3, 4, 5 would be sent as `0x20 0x01 0x05 0x00 0x00 0x00 0x01 0x02 0x03 0x04 0x05`.

To send a map, send `0x21`, followed by the type encoding of the key type, followed by the type encoding of the value type, followed by the number of key-value pairs as an unsigned little-endian 32-bit integer, followed by the key-value pairs without type encodings (except for variable-length types such as optionals, strings, lists and maps, which must include their full type encodings before each key or value). For example, the map of unsigned 8-bit integers to strings `{0x30 => "", 0x40 => "Hi", 0x50 => "This is a long string"}` would be sent as `0x21 0x01 0x1f 0x03 0x00 0x00 0x00 0x30 0x10 0x40 0x12 0x48 0x69 0x50 0x1f 0x15 0x00 0x00 0x00 0x54 0x68 0x69 0x73 0x20 0x69 0x73 0x20 0x61 0x20 0x6c 0x6f 0x6e 0x67 0x20 0x73 0x74 0x72 0x69 0x6e 0x67`.

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
