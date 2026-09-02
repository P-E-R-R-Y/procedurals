# procedurals

A deterministic, header-mostly C++20 library for procedural generation — noise, sampling, rule systems, and dungeon/world primitives.


Same input + seed → same output. No global RNG, no `<random>` dependency.

| fBm + ridge + warp | TinyKeep dungeon | Wave Function Collapse |
|:---:|:---:|:---:|
| ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/final.png) | <img src="https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/visualizer_tinykeep.png" width="512"/> | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/wave_func_collapse.png) |

---

## Features

- Hash-based deterministic PRNG (`random0D/1D/2D/3D/4D`)
- Value noise, fBm, ridge, domain warp, Worley, Simplex
- Easing / quantize / rarity helpers
- Poisson-disk sampling (Bridson)
- Wave Function Collapse with user-defined adjacency rules
- BSP dungeons, drunkard walk, cellular automata
- TinyKeep room layout: scatter → relax → Delaunay → MST + loopback
- PPM visualizer for quick debug renders

---

## Build & test

Requirements: a C++20 compiler, CMake ≥ 3.14, GoogleTest discoverable by `find_package` (the `cmake/` folder provides finders for `System`, `Maths`, and `GoogleTest`).

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

This produces `build/libprocedurals.a` and `build/procedurals_tests`. The tests dump `.ppm` images into the working directory — open them with Preview (macOS), GIMP, `feh`, or any PPM-capable viewer.

---

## Use in your own project

### Via `FetchContent`

```cmake
include(FetchContent)
FetchContent_Declare(procedurals
    GIT_REPOSITORY https://github.com/p-e-r-r-y/procedurals.git
    GIT_TAG        main)
FetchContent_MakeAvailable(procedurals)

target_link_libraries(my_app PRIVATE procedurals)
```

### Via submodule

```bash
git submodule add https://github.com/p-e-r-r-y/procedurals.git third_party/procedurals
```

```cmake
add_subdirectory(third_party/procedurals)
target_link_libraries(my_app PRIVATE procedurals)
```

Then include whichever headers you need:

```cpp
#include "Deterministic.hpp"
#include "Noise.hpp"
#include "TinyKeepDev.hpp"
#include "PoissonDisk.hpp"
#include "WaveFunctionCollapse.hpp"
```

---

## What's in the box

| Header | Provides |
|---|---|
| `Deterministic.hpp` | `random0D/1D/2D/3D/4D`, `mix64`, `SplitSeed` |
| `Smoothing.hpp`     | `Lerp`, `Clamp`, `EaseIn/Out/InOut`, `Quantize`, `RareEvent` |
| `Noise.hpp`         | `noise2D`, `fbm2D`, `ridge2D`, `warp2D`, `worley2D`, `simplex2D`, `rarity2D` |
| `Visualizer.hpp`    | `visualizer2D(...)` PPM writers (callback, grid, points) |
| `TinyKeepDev.hpp`   | Room scatter + relax + Bowyer-Watson Delaunay + `mst()` + `mstWithLoopback()` |
| `PoissonDisk.hpp`   | Bridson's algorithm — evenly-spaced point sets |
| `WaveFunctionCollapse.hpp` | Tile-based WFC with user-defined adjacency rules |

---

## Examples

### Deterministic random per coordinate

```cpp
#include "Deterministic.hpp"

float v = random2D(x, y, /*seed=*/42);   // in [0, 1], stable per (x, y, seed)
```

### 2D noise and fBm

```cpp
#include "Noise.hpp"

float n = noise2D(x * 0.05f, y * 0.05f, seed);
float h = fbm2D  (x * 0.01f, y * 0.01f, /*octaves=*/5, seed);
```

| noise2D | fbm2D |
|:---:|:---:|
| ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/noise.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/fbm.png) |

### Layered terrain

```cpp
visualizer2D("terrain.ppm", 512, 512, 50.0f, [](float x, float y) {
    float terrain = fbm2D(x, y, 5);
    float warped  = warp2D(x, y, 1, 2.0f);
    return Quantize<3>(terrain * 0.5f + warped * 0.5f);
});
```

![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/final.png)

### Splitting a seed into independent channels

```cpp
uint64_t base   = 12345;
uint64_t height = SplitSeed(base, 0);
uint64_t biome  = SplitSeed(base, 1);
uint64_t loot   = SplitSeed(base, 2);
// each channel is independent but reproducible from base
```

### TinyKeep-style dungeon (full pipeline)

```cpp
#include "TinyKeepDev.hpp"

auto rects     = TinyKeepDev::generateRects<50>(200.0f, 0.5f, {0.25f, 0.75f}, 42);
TinyKeepDev::relaxRects<100>(rects);
auto delaunay  = TinyKeepDev::bowyerWatson(rects);
auto mst       = TinyKeepDev::mst(delaunay);
```

| 1. Scatter | 2. Relax | 3. Delaunay | 4. MST |
|:---:|:---:|:---:|:---:|
| ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/tinykeep_1_generate.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/tinykeep_2_relax.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/tinykeep_3_delaunay.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/tinykeep_4_mst.png) |

### Poisson disk sampling

```cpp
#include "PoissonDisk.hpp"

auto points = PoissonDisk::generate(/*W=*/200, /*H=*/200,
                                    /*minDist=*/10.0f,
                                    /*k=*/10, /*seed=*/123);
```

![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/poisson_disk.png)

### Wave Function Collapse

```cpp
#include "WaveFunctionCollapse.hpp"

WaveFunctionCollapse::Rules rules{{
    {0, 1},      // tile 0 next to: 0, 1
    {0, 1, 2},   // tile 1 next to: 0, 1, 2
    {1, 2, 3},   // tile 2 next to: 1, 2, 3
    {2, 3}       // tile 3 next to: 2, 3
}};

auto grid = WaveFunctionCollapse::collapse(64, 64, rules, /*seed=*/0);
```

![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/wave_func_collapse.png)

### BSP dungeon

The `tests/TestBSP.cpp` showcase recursively splits a rectangle and connects room centers with corridors:

![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/bsp_dungeon.png)

### More patterns

| Worley | Inv. Worley | Ridge |
|:---:|:---:|:---:|
| ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/worley.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/inv_worley.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/ridge.png) |

| Domain warp | Simplex | Rarity |
|:---:|:---:|:---:|
| ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/domain_warp.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/simplex.png) | ![](https://raw.githubusercontent.com/P-E-R-R-Y/procedurals/main/docs/preview/rarity.png) |

---

## Project layout

```
procedurals/
├── includes/      public headers (the actual library)
├── sources/       placeholder TU so CMake has something to compile
├── tests/         GoogleTest cases + PPM visualizers
├── cmake/         Find modules (GoogleTest, System, Maths)
├── docs/          Doxyfile, roadmap, preview/ images
└── CMakeLists.txt
```

---

## Design principles

- **Deterministic.** Output is a pure function of `(coords…, seed)`.
- **Header-mostly.** Algorithms live in the headers; `libprocedurals.a` exists so CMake has a target to link.
- **No global state.** No `srand`, no `<random>`, no thread-local RNG.
- **Composable.** Functions take coordinates, return floats — chain them.
- **Domain-agnostic.** No assumptions about tiles, rooms, biomes, or game objects.

See [Roadmap.md](Roadmap.md) for planned phases.

---

## Generating Doxygen docs

```bash
doxygen Doxyfile
```

---

## Author

Perry Chouteau — perry.chouteau@outlook.com
