# procedurals

A small, header-only C++20 library of deterministic procedural-generation
primitives — noise, smoothing/easing, hash-based pseudo-random, and a few
higher-level helpers (Poisson-disk sampling, BSP, drunkard walk, L-systems,
Wave Function Collapse, TinyKeep-style room layout).

Same input + same seed → same output, always. No global RNG state, no
dependency on `<random>`.

> The library is research-oriented: I'm building it to experiment with
> procedural worlds (dungeons, terrains, scattering). The API is not stable
> yet — see [Roadmap.md](Roadmap.md).

---

## What's in the box

| Header | What it gives you |
|---|---|
| `Deterministic.hpp` | Hash-based PRNG: `random0D/1D/2D/3D/4D(coords, seed)`, `mix64`, `SplitSeed`. |
| `Smoothing.hpp` | `Lerp`, `Clamp`, `EaseIn/Out/InOut`, `Quantize`, `RareEvent`. |
| `Noise.hpp` | `noise2D` (value noise), `fbm2D`, `ridge2D`, `warp2D`, `worley2D`, `simplex2D`, plus a PPM `visualizer2D` helper. |
| `TinyKeepDev.hpp` | TinyKeep-inspired room generator — scatter rectangles in a disc, then relax them apart. |

The algorithmic showcases (BSP dungeon, cellular automata, drunkard walk,
L-system, Poisson-disk, WFC) currently live in the [tests/](../tests) folder.
They run as GoogleTest cases and dump PPM images of their output, which is the
fastest way to see what each one produces.

---

## Build

Requirements: a C++20 compiler, CMake ≥ 3.14, and GoogleTest discoverable by
`find_package` (the `cmake/` folder provides the finders).

```bash
# from the repo root
cmake -S . -B build
cmake --build build -j
```

This produces:

- `build/libprocedurals.a` — the static library (currently just a placeholder
  object; everything useful is in the headers).
- `build/procedurals_tests` — the test runner.

### Run the tests

```bash
ctest --test-dir build --output-on-failure
# or, to see the GTest output directly:
./build/procedurals_tests
```

Several tests write `.ppm` images to the working directory (`noise.ppm`,
`fbm.ppm`, `bsp_dungeon.ppm`, `wfc_test.ppm`, …). Open them with any image
viewer that handles PPM — Preview on macOS, GIMP, `feh`, etc.

---

## Using it in your own project

It's a static library + headers. Drop the repo in as a submodule (or
`FetchContent`) and link against `procedurals`:

```cmake
add_subdirectory(third_party/procedurals)
target_link_libraries(my_app PRIVATE procedurals)
```

Then include whichever headers you need:

```cpp
#include "Noise.hpp"
#include "Smoothing.hpp"
#include "TinyKeepDev.hpp"
```

---

## Quick examples

### Deterministic random per coordinate

```cpp
#include "Deterministic.hpp"

float v = random2D(x, y, /*seed=*/42); // in [0, 1], stable for the same (x,y,seed)
```

### 2D noise + fBm

```cpp
#include "Noise.hpp"

float n  = noise2D(x * 0.05f, y * 0.05f, seed);
float h  = fbm2D (x * 0.01f, y * 0.01f, /*octaves=*/5, seed);
```

### Dump a noise field to a PPM image

```cpp
visualizer2D("terrain.ppm", 512, 512, /*scale=*/50.0f,
    [](float x, float y) {
        return fbm2D(x, y, 5);
    });
```

### Splitting a seed into independent channels

```cpp
uint64_t base   = 12345;
uint64_t height = SplitSeed(base, 0);
uint64_t biome  = SplitSeed(base, 1);
uint64_t loot   = SplitSeed(base, 2);
// each channel is independent but reproducible from `base`
```

### TinyKeep-style room scatter

```cpp
#include "TinyKeepDev.hpp"

auto rects = TinyKeepDev::generateRects</*nbRects=*/120>(
    /*radius=*/100.0f,
    /*maxRectRatio=*/0.5f,
    /*radiusRatios=*/{0.1f, 1.0f},
    /*seed=*/1
);
```

---

## Project layout

```
procedurals/
├── includes/        public headers (the actual library)
├── sources/         placeholder TU so CMake has something to compile
├── tests/           GoogleTest cases + PPM visualizers
├── cmake/           Find modules (GoogleTest, System)
├── docs/            this readme + roadmap + Doxyfile
└── CMakeLists.txt
```

`main.cpp` at the repo root is a scratch file used while iterating on
rectangle relaxation — it is **not** part of the CMake build. Compile it
standalone if you need to poke at it:

```bash
c++ -std=c++20 main.cpp -o relax && ./relax
```

---

## Design principles

- **Deterministic.** Output is a pure function of `(coords…, seed)`.
- **Header-mostly.** Algorithms live in the headers; the `.a` exists so CMake
  has a target to link.
- **No global state.** No singletons, no `srand`, no thread-local RNG.
- **Composable.** Functions take coordinates and return floats — chain them.
- **Domain-agnostic.** No assumptions about terrain, tiles, or game objects.

See [Roadmap.md](Roadmap.md) for the planned phases (core → patterns →
combinators → sampling → rules).

---

## Generating Doxygen docs

```bash
doxygen docs/Doxyfile
```

Output lands in `docs/preview/` (already gitignored).

---

## Author

Perry Chouteau — <perry.chouteau@outlook.com>
