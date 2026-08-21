# sif — render engine, asset system and UI layout engine

**Author:** Daniil Sukhovii
**Student number:** s0240228
**Repository:** [click](https://github.com/sukhoviidaniil/sif)

Used by [Bomberman](https://github.com/sukhoviidaniil/Bomberman_2025_2026) (Advanced Programming 2025-2026), fetched
automatically at configure time - see that repository for the game itself.

A backend-agnostic C++20 engine: an asset system (loading, queueing, hot references),
an XML-driven UI layout engine, a retained-free render pipeline and an audio
interface — plus one SFML backend and one CPU-only (headless) backend that implement
those interfaces.

The engine itself links **no graphics or audio library**. `sif_lib` is the only target
built unconditionally; every SFML-consuming target (the reference backend, the asset
tools, the demo) is opt-in and finds SFML for itself, lazily, only when actually enabled.

---

## Layout

Every target sif defines lives under one of two top-level folders — the way SFML's own
repository keeps `sfml-system`, `sfml-window`, `sfml-graphics`, `sfml-audio` and
`sfml-network` together under `src/SFML/` regardless of which one you're building:

```
sif/                     everything that IS sif
  include/sif/
    asset/                AssetRegistry, AssetRecord, AssetHandle, per-type asset interfaces
    audio/                AudioPlayer interface
    diagnostics/          Logger, include-cycle analysis (PlantUML export)
    event/                Event_Bus, Observer, Event_Store, event definitions
    internal/             GUID, RecordID, Rect, Color, Size, Random, Delta_Timer
    io/                   JSON helpers
    layout_engine/        Tokenizer -> Parser -> UIFactory -> UIElement tree
    math/                 Vector, Matrix, Vector2, Point2, DirectedGraph
    render/               RenderFrame, RenderNode visitor tree, Renderer interface,
                          Camera (normalized world [-1,1] -> pixels), RB_Config
  src/                    implementations
  test/                   self-registering test suite (no external framework, target sif_tests)
  tools/                  sif_sprite_packer (needs SFML for image I/O; Asset_GUID_Assignment
                          and Asset_Reference_Serialization live in sif/ itself - no SFML)
  backends/               sif_sfml: the reference SFML backend (renderer, audio player,
                          event collector, one asset loader per asset type) plus the
                          CPU-only headless variant and the Graphics_Factory that
                          switches between them by ast::RB_Type

app/                      sif_demo - a *consumer* of sif, not part of it (like an
                          "examples/" folder in most libraries): four windowed demos
                          plus the headless verification run, linking sif_sfml

cmake/GetSFML.cmake       finds/fetches SFML 2.6 for sif's own targets only (see below)
data/                     assets, asset descriptors, authoring scenes and serialized scenes
uml/                      PlantUML diagrams of the subsystems
```

All of it also lands in one place at *build* time: `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`,
`CMAKE_LIBRARY_OUTPUT_DIRECTORY` and `CMAKE_RUNTIME_OUTPUT_DIRECTORY` are set once, in the
root `CMakeLists.txt`, so every library ends up in `build/lib/` and every executable in
`build/bin/` regardless of which source subdirectory produced it — again exactly how
building SFML gives you one `lib/` folder, not one per `src/SFML/*` subdirectory.

---

## Building

Requirements: CMake ≥ 3.20, a C++20 compiler, and network access on the *first* configure
only. `nlohmann/json` is not something you provide: `cmake/GetJSON.cmake` fetches the
single header itself, straight from nlohmann/json's own GitHub release assets, verified
against a pinned SHA-256, the first time `external/json/json.hpp` is not already there —
after that first configure it is reused with no further network access. SFML 2.6 is needed
only for the targets that ask for it (see below) and, if none of them are enabled, is never
searched for at all.

If you already have a copy (vendored, or from a package manager) you would rather use
instead of fetching one, either drop it at `external/json/json.hpp` yourself, or point at
it with `-DSIF_JSON_SOURCE=/path/to/json.hpp` (skips both the download and the hash check —
it is your copy at that point).

Verified on the reference platform: **Ubuntu 24.04, GCC 13, SFML 2.6.1, CMake 3.28**.

```bash
# engine + tests only - no SFML search happens at all
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DSIF_BUILD_TOOLS=OFF -DSIF_BUILD_SFML_BACKEND=OFF -DSIF_BUILD_DEMO_APP=OFF
cmake --build build -j
./build/bin/sif_tests

# the usual case: engine + asset tools + the reference backend (sif_sfml) - what a
# consumer like a game built on sif actually needs; still no demo window
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# everything, including the demo application
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSIF_BUILD_DEMO_APP=ON
cmake --build build -j
./build/bin/sif_demo headless data/
```

The build is warning-clean under `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`.

### What gets built, and how it finds SFML

| Option | Default | What it controls | If SFML can't be found/fetched |
|---|---|---|---|
| `SIF_BUILD_TOOLS` | `ON` | `Asset_GUID_Assignment`, `Asset_Reference_Serialization` (no SFML), `sif_sprite_packer` (needs SFML) | `sif_sprite_packer` is skipped with a `STATUS` message; the other two are unaffected |
| `SIF_BUILD_SFML_BACKEND` | `ON` | `sif_sfml` - the reference backend a consumer actually links against | configure fails with `FATAL_ERROR` and a clear message - this target is not optional once requested |
| `SIF_BUILD_DEMO_APP` | `OFF` | `sif_demo` | skipped with a `STATUS` message |

Enabling none of the three means `cmake/GetSFML.cmake` is never even `include()`-d - a pure
engine build touches no graphics library, searches for none, and fetches none.

### Consuming sif from another project (e.g. via `FetchContent`)

A project that only wants `sif_sfml` needs nothing beyond `FetchContent_MakeAvailable(sif)`
and `target_link_libraries(your_target PRIVATE sif_sfml)` - SFML comes along transitively.
**No SFML target has to be prepared or passed in beforehand**: sif finds or fetches its own,
using only sif-prefixed inputs (`SIF_SFML_DIR`, not the generic `SFML_DIR`) and a directory
search that never consults `CMAKE_PREFIX_PATH` or environment variables a consumer might have
set for an unrelated purpose.

That isolation is deliberately two-way and holds even when the *same* configure also searches
for a different SFML version for the consumer's own, separate use (SFML 2 for sif's backend,
SFML 3 for the consumer's own code, say) — verified directly: a sibling project with its own
`find_package(SFML 3...)` and sif's own search were run in the same configure, with the
generic `SFML_DIR` cache variable already pointing at the 3.x install; sif still resolved its
own 2.6.1 correctly, and the sibling's `SFML_DIR` cache entry came out the other side
untouched. The one thing this cannot cover — CMake's own limitation, not a gap left
unfixed — is if the *consumer's own* search runs at a CMake scope that is an **ancestor** of
sif's (e.g. the consumer's own root `CMakeLists.txt`, before it fetches sif): a target named
`sfml-graphics` created there is then visible to sif's search the same way it is to
everything else the consumer's root goes on to configure, and sif's own module detects this
specific situation and prints a `WARNING` naming it rather than silently linking against a
possibly-incompatible version. Keeping a consumer's own SFML search inside one of its own
subdirectories (a sibling of wherever it fetches sif, not its root) avoids this entirely, and
is the pattern used by the Bomberman project this engine was built alongside.

---

## The asset pipeline

Assets are described by `*.asset.json` files that live next to the data they describe.
Two command-line tools turn those into what the runtime consumes:

```bash
# 1. scan data/, assign/collect GUIDs, write the registry
./build/bin/Asset_GUID_Assignment data/ data/bin/registry.rgst.json

# 2. rewrite authoring scenes (asset_name="...") into serialized ones (guid="...")
./build/bin/Asset_Reference_Serialization data/bin/registry.rgst.json data/scenes data/bin/scenes
```

Authoring files stay human-readable (`asset_name="DemoBall"`); the runtime only ever sees
stable numeric GUIDs, so renaming an asset cannot silently break a scene.

Supported asset types: `Font`, `SpriteSingle`, `SpriteAtlas`, `SpriteGrid`,
`PrimitiveAnimation`, `Sound`.

If per-frame artwork needs packing into strips (`PrimitiveAnimation` addresses frames as
rectangles inside one texture), `sif_sprite_packer` does that from a small JSON description -
see `sif/tools/Sprite_Packer.cpp` for the format.

---

## Running the demos

Requires `-DSIF_BUILD_DEMO_APP=ON` (see above; off by default).

```bash
./build/bin/sif_demo <demo-id> [data-dir]     # data-dir defaults to ../data/
```

| id | what it shows |
|----|---------------|
| `sprites`   | `SpriteSingle`, `SpriteAtlas` addressed by record **name**, `SpriteGrid` addressed by flat record **id** — all through one `ui::Sprite` element |
| `animation` | one `PrimitiveAnimation` asset driving four independent cursors (0.5x / 1x / 2.5x / paused); `SPACE` pauses the 4th, `S` restarts all |
| `sound`     | three `Sound` assets through `audio::AudioPlayer`; `1` blip, `2` looping chime toggle, `3` thud, `↑`/`↓` master volume — bars are driven by real voice state, not by the key press |
| `all`       | everything at once, synchronised by the animation cursor: a sound fires when the animation wraps, the atlas icon advances on the same beat, the tile strip lights up with the current frame |
| `headless`  | no window, no audio device: loads every asset, checks geometry and timing, exits non-zero on failure (this is what CI runs) |

`ESC` closes any windowed demo.

---

## Tests

`sif/test` is a small self-registering suite: adding a case is one `SIF_TEST(...)` block
in any `*Tests.cpp`, with nothing to wire up elsewhere.

```bash
./build/bin/sif_tests
```

Covered: asset registry queueing/concurrency/stale-attempt handling, `Rect` arithmetic,
animation frame timing (looping and non-looping), UI element lookup/menu behaviour, the
`Sprite`/`Animation` elements' render output, parser error handling, singleton identity,
and the `Camera` projection (all three aspect policies, zoom, centring, sub-viewports,
y-axis flip, round-tripping and culling).

## Coordinates

`sif::rnd::Camera` is the single place where normalized world coordinates become pixels.
The world is bounded by `[-1, 1]` on both axes, so game logic never depends on the
window size; `Camera` turns a world point, size or rectangle into screen space according
to a viewport, an aspect policy (`Stretch` / `Fit` / `Fill`), a zoom and a centre. It
contains no graphics-library code and is unit-tested without a window.

Note that the UI layout engine works directly in screen pixels — a HUD is authored
against the window, not against the world. The two systems meet only where a scene draws
both: world content goes through `Camera`, UI content does not.

---

## CI

`.circleci/config.yml` runs two jobs: one builds and tests the engine with every
SFML-consuming target explicitly disabled (**no SFML installed at all** on that machine),
the other installs SFML, builds everything including the opt-in demo, and runs the
headless asset + animation + sound verification.
