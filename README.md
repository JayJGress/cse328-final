# CSE328 — Portal Raycaster

**Jay Gress**
**115469091**

A 2.5D raycasting engine written in C using SDL2, inspired by classic renderers like Wolfenstein 3D. This project extends standard raycasting with a portal system that restructures physical space, allowing seamless traversal into non-adjacent environments.

---

## Features

- **Raycasting renderer** - per-column DDA raycasting with distance-based shading
- **Portal system** - portals link separate rooms (cells); rays and the player pass through them seamlessly
- **Portal rotation & translation** - portals can be repositioned and rotated at runtime
- **Same-room portals** - portals can link two locations within the same cell
- **Minimap overlay** - top-left minimap shows the current cell, portal positions, and player direction
- **Portal frame overlay** - optional debug view highlights portal edges and minimap portal lines (toggle with `F`)

---

## Dependencies

- GCC
- [SDL2](https://www.libsdl.org/) (`libsdl2-dev` on Debian/Ubuntu)
- math library (`libm`, included via `-lm`)

Install SDL2 on Debian/Ubuntu:
```
sudo apt install libsdl2-dev
```

---

## Building

```
make
```

The binary is placed in `bin/raycaster`. Object files are written to `obj/`.

To clean all build artifacts:
```
make clean
```

---

## Controls

| Key | Action |
|-----|--------|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Rotate left / right |
| `F` | Toggle portal frame overlay |
| `Tab` | Cycle selected portal (for editing) |
| `↑ ↓ ← →` | Translate selected portal |
| `<` / `>` (`,` / `.`) | Rotate selected portal |

---

## How It Works

### Raycasting
For each screen column a ray is cast from the player's position using the DDA algorithm. The perpendicular wall distance is used to compute the height of the wall slice drawn for that column, producing the 3D perspective effect.

### Portal Traversal
Each cell holds a list of portals (line segments with a linked partner in another cell). During raycasting, if a ray intersects a portal before hitting a wall, it is rotated and repositioned at the partner portal and continues casting in the destination cell. The player uses the same crossing logic for movement between rooms.

### Portal Linking
When two portals are linked, a `rotationAngle` is computed from the difference in their facing angles. This angle is applied to redirect rays and rotate the player's orientation on crossing, so the transition feels spatially consistent regardless of the angle between the two portals.

---

## Adding Cells and Portals
 
All world setup lives in `src/cell.c` inside `init_world()`.
 
### 1. Define a Cell Map
 
A cell is an `11 x 8` tile grid. `0` is open space, anything non-zero is a wall. Wall tile values map to colours in the renderer:
 
| Value | Colour |
|-------|--------|
| `1` | Grey |
| `2` | Red |
| `3` | Blue |
| `4` | Green |
 
Add your map and copy it into a cell slot:
 
```c
int map2[CELL_HEIGHT][CELL_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1}
};
memcpy(cells[2].map, map2, sizeof(map2));
cells[2].portalCount = 0;
```
 
The cell index must be less than `MAX_CELLS` (currently `8`).
 
### 2. Create Portals
 
Use `portal_init` to place a portal in a cell:
 
```c
portal_init(Portal *p, Cell *cell, double cx, double cy, double angle);
```
 
- `p`: pointer to a portal slot in the cell (`&cells[n].portals[i]`)
- `cell`: pointer to the owning cell (`&cells[n]`)
- `cx`, `cy`: center position of the portal in tile coordinates
- `angle`: facing direction in degrees (the direction a player walks *through* the portal)
 
```c
cells[2].portalCount = 1;
Portal *pA = &cells[2].portals[0];
portal_init(pA, &cells[2], 5.0, 4.0, 180);
```
 
Each portal counts toward its cell's `portalCount`, which must be set before use and must not exceed `MAX_PORTALS` (default `16`).
 
### 3. Link Portal Pairs
 
Use `portal_link` to connect two portals:
 
```c
portal_link(Portal *a, Portal *b);
```
 
This is bidirectional — `a` exits into `b`'s cell and vice versa. The rotation angle between them is computed automatically.
 
```c
Portal *pB = &cells[0].portals[2];
portal_init(pB, &cells[0], 9.0, 4.0, 0);
cells[0].portalCount++;
 
portal_link(pA, pB);
```
 
### Same-Room Portals
 
Portals can link two positions within the same cell. Just pass the same cell pointer to both `portal_init` calls and link them normally:
 
```c
Portal *p1 = &cells[0].portals[0];
Portal *p2 = &cells[0].portals[1];
portal_init(p1, &cells[0], 2.0, 2.0, 90);
portal_init(p2, &cells[0], 8.0, 6.0, 270);
cells[0].portalCount = 2;
portal_link(p1, p2);
```

---

## Project Structure

```
CSE328-FINAL/
├── include/
│   ├── cell.h       # Cell and Portal struct definitions
│   ├── player.h     # Player struct and input/movement interface
│   ├── portal.h     # Portal math (init, link, intersection)
│   └── render.h     # Renderer interface and screen constants
├── src/
│   ├── main.c       # SDL setup and game loop
│   ├── cell.c       # World definition (maps, portal placement)
│   ├── player.c     # Player movement, rotation, portal crossing
│   ├── portal.c     # Portal geometry, linking, ray intersection
│   └── render.c     # Raycasting, wall drawing, minimap, overlays
├── bin/             # Compiled binary (generated)
├── obj/             # Object files (generated)
└── Makefile
```