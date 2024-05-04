# Overview
For this final project, I wanted to modify our pokemon game and turn it into a more Zelda-like RPG. To me, that means changing the design to be more conducive to exploration.

### World Generation
The first thing I decided to implement was different types of maps. Now each map is given a "geotype" that represents the sort of geography in the region, and the terrain of a map varies based on its geotype. This means some parts of the world will be covered in water and have few to no boulders or mountains, some will be dominated by mountains, etc.

Geotypes are assigned with a modified version of the `map_terrain()` algorithm, where different types expand from a number of seeds.

Pressing `m` during the game pauses it, much like `t` or `B`, and brings up a scaled-down representation of the world map on the right side. Different geotypes are shown with a character, most of which are shared with the character symbol for the type of terrain that dominates that geography.
