# Known bugs and suspicious code

Found while fixing includes and adding the tree editor (2026-11-14).

## Open

### 1. `CapsuleBlob` hides `potential` and `threshold`: it reads values that are never set
`src/blobs/include/Blob.h`: `CapsuleBlob` declares its own `potential` and `threshold`
members, which hide the ones in `Blob`. The constructor passes the values to
`Blob(potential, threshold)`, so only the base copies are set.
`CapsuleBlob::Distance` uses `potential`, which resolves to the uninitialized `CapsuleBlob` copy.
Capsule results therefore depend on whatever happens to be in memory.
**Fix:** delete the two member declarations from `CapsuleBlob`.

### 2. `CapsuleBlob::Distance` subtracts `potential` from a squared distance
`src/blobs/Blob.cpp`: `return SquaredNorm(pa - ba*h) - potential;`
This mixes the SDF capsule formula (`length(...) - radius`) with a squared distance.
The result can go negative and no longer means the same thing as `SphereBlob::Distance`,
which returns a plain squared distance. The same `potential` is used again as the
Gaussian's sigma in `Blob::Evaluate`, so it ends up applied twice.
**Fix:** probably return `SquaredNorm(pa - ba*h)`, matching the sphere.

### 3. `Blob::Evaluate` passes a squared distance to `gaussian`, which squares it again
`src/blobs/Blob.cpp`: `gaussian(Distance(p), potential)`. `Distance` returns d², and
`gaussian` computes `exp(-x*x / 2σ²)`, so the falloff is effectively `exp(-d⁴ / 2σ²)`.
That might be intended, but it is unusual.
The Gaussian is also normalized by `1/sqrt(2πσ²)`, so its peak value changes with `potential`.
With a fixed `threshold`, that changes the blob's size in a non-obvious way
(a large `potential` can push the peak below `threshold`, and then the blob disappears).

### 4. `wyvill` is not clamped beyond distance 1
`src/blobs/include/Blob.h`: `pow(1 - d², n)` is negative for `d² > 1` when `n` is odd,
and grows again for even `n`. The Wyvill kernel should be `0` when `d² >= 1`.
(Not used yet.)

### 5. Operator `GetLipschitzBound` / `ComputeBBox` throw string literals
`src/blobs/OperatorBlob.cpp`: `throw("not implemented")` throws a `const char*`, not a
`std::exception`, so a `catch (const std::exception&)` won't catch it.
Prefer `throw std::logic_error("not implemented");`.
The leaf versions are silent no-ops, and `Node`'s bounding box is always the zero box.

### 6. `BBox` corner naming vs. `isIn`
`src/include/BBox.h`: the corners are named `left_up` and `right_down`. In a Y-up world,
"up" suggests the max Y, but `isIn` (`src/BoundingBox.cpp`) treats `left_up` as the
min corner on every axis. Rename to `min`/`max`, or check that callers pass them in that order.

### 7. Operator nodes don't own their children
`OperatorBlob` keeps raw `Node*` children and never deletes them. Trees built with
`new` (as `main.cpp` used to) leak. The GUI parser avoids this by keeping every node in
`BlobTree::nodes`, but code that builds trees by hand still has to manage memory itself.

### 8. ESC quits the app, even while typing in the editor
`WindowShouldClose()` returns true on ESC (raylib's default exit key).
`SetExitKey(KEY_NULL)` would disable that.

### 9. `make test` fails because there are no tests
The Makefile now links raylib into the test binary, so `make test` builds,
but `test/test_main.cpp` has no test cases, so Catch2 exits with "No tests ran" (exit code 2).

## Already fixed

- **Include case mismatches**: `#include "blob.h"` and `"node.h"` worked on macOS
  (case-insensitive file system) but would fail on Linux.
- **`#pragma once` in .cpp files**: `Blob.cpp`, `OperatorBlob.cpp`, `implicits.cpp`.
- **Missing `<algorithm>`** for `std::clamp` in `Blob.cpp`.
- **Missing definitions** (link errors): `Node::Node()`, `MoreVector::Dot`, `BBox` constructor and `isIn`.
- **Old `Cross` in `MoreMath.h`** returned `Vector(cx, cz, cy)` (y and z swapped).
  The old free `Dot`/`Cross` also used `u.X`, which is the static constant
  `Vector::X == (1,0,0)`, not the x coordinate.
- **Operator subclasses inherited privately** from `OperatorBlob`, so they couldn't be used as `Node*`.
- **`make test` failed to link** once the GUI was added, because the test binary links
  all app objects but not raylib.
- **Stale `build/blobs/blob.o`** from before the rename to `Blob.cpp` was still linked.
  Run `make clean` after renaming source files.
