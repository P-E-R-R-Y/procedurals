# 🧮 procedurals

A **deterministic procedural math library** for C++ —  
pure functions for noise, sampling, combinators, and rule systems.  
No OOP, no “game generators,” just **mathematical composition**.

---

## 📁 Structure

procedurals/
├── core/          # random, hash, math, fields
├── combinators/   # blend, warp, remap, mask
├── sampling/      # Poisson, blue noise, random grids
├── rules/         # grammars, L-systems
├── patterns/      # noise, fractals, procedural patterns
├── include/       # public headers
└── tests/         # deterministic tests

---

## 🧭 Roadmap

### **Phase 1 — Core**
- `random.h`: hash-based random (`random01`, `split`)
- `hash.h`: coordinate hashing
- `math.h`: helpers (`lerp`, `clamp`, `smoothstep`, `Vec2/3`)
- `field.h`: sampled data grids

### **Phase 2 — Patterns**
- Value, Perlin, Simplex, Worley noise
- fBm, turbulence, ridged variants

### **Phase 3 — Combinators**
- Blend, warp, remap, mask, normalize

### **Phase 4 — Sampling**
- Poisson disk, blue noise, random grid

### **Phase 5 — Rules**
- Grammar systems, L-systems

### **Phase 6 — Patterns++
- High-level math constructs (islands, marble, clouds)

### **Phase 7 — Tests / Include**
- Determinism tests, public `procedurals.h` API

---

## 🧩 Priorities

| Component | Priority |
|------------|-----------|
| `core/random.h` | 🔥 must-have |
| `core/hash.h` | 🔥 must-have |
| `core/math.h` | ✅ early |
| `patterns/noise.h` | ✅ early |
| `combinators/` | ✅ mid |
| `sampling/` | ⏳ later |
| `rules/` | ⏳ later |
| `tests/` | 🔥 must-have |

---

## 💡 Philosophy

| Principle | Meaning |
|------------|----------|
| Functional | Pure functions, no mutable state |
| Deterministic | Same input → same output |
| Composable | Chainable math operations |
| Domain-agnostic | No terrain/AI logic |
| Minimal | Header-only, fast, portable |

---

## 🧱 Example

```cpp
using namespace procedurals;

float v = remap(
  blend(fbm2D(x, y, seed),
        worley2D(x, y, split(seed, 1)), 0.4f),
  -1, 1, 0, 1
);
```

## Next Steps

- Cellular automata
- Drunkard Walk /Random Walk
- Voronoi / Worley noise
- Wave function collapse
- Poison Disk sampling
- reaction-diffusion