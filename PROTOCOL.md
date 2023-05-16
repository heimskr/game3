# Packets

Packets are encoded as a little-endian 2-byte integer representing the packet type, followed by a little-endian 4-byte integer representing the payload length.

<!-- TODO: message encoding -->

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
	- `i32[]` Chunk Positions

	The chunk positions will be sent as interlaced (x, y) pairs.

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

	- `u64` Command ID
	- `string` Command

7. **Self Teleported**: tells a client the position of their player.

	- `i32` Realm ID
	- `{i64,i64}` Position

8. **Chunk Tiles**: sends all the terrain data for a chunk to a client.

	- `i32` Realm ID
	- `{i32,i32}` Chunk Position
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