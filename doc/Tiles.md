# Tiles

Tiles are the different textures that can appear on the ground in the world.
They're rendered by the ElementBufferedRenderer and UpperRenderer.
Every tile has a corresponding directory under `/resources/tileset` with two
files: `tile.png`, which contains all the texture data, and `tile.json`, which
contains metadata about the tile.

## `tile.png`

Texture data can come in one of these forms: single, tall single, 4x4 autotile, tall 4x4 autotile or 8x8 autotile.
Single sprites are simple 16 pixel by 16 pixel textures that are rendered at one position.
Tall single sprites are like single sprites, but they also have an upper portion that's rendered after all other tiles
one tile north of the base position. 4x4 autotile textures are 64 pixel by 64 pixel atlases that contain 16 different
textures that are chosen via a 4-way marching squares algorithm. Tall 4x4 autotile textures are 64 pixel by 128 pixel
atlases that are like regular 4x4 autotiles, but each of the 16 possibilities has an upper portion as well.
8x8 autotiles contain 47 possibilities chosen via an 8-way marching squares algorithm.

## Autotiling

Autotiling is when a tile's appearance is influenced by the adjacent tiles.
A marching squares algorithm is used to determine which tile variant to use
based on which directions there are "matching" neighbors. Autotiling information
needs to be encoded in `tile.json`.

## `tile.json`

`tile.json` files contain metadata that the engine uses to categorize tiles.
It contains an object with these fields:

- `credit`: a string crediting the artist of the texture data.
- `solid`: a boolean indicating whether the tile should prevent entities from moving to its position.
- `categories`: a list of identifier strings of all the categories that apply to the tile.
- `autotile`: a boolean indicating whether the tile participates in autotiling. This is false by default.
  It should be `true` if the tile isn't a single tile or tall single tile.
- `land`: a vestigial boolean indicating whether the tile is a "land" tile.
  This categorization is no longer useful, so it's not required anymore.