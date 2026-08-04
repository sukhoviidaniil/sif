# sif — render engine, asset system and UI layout engine

**Author:** Daniil Sukhovii
**Student number:** s0240228

A backend-agnostic C++20 engine: an asset system (loading, queueing, hot references),
an XML-driven UI layout engine, a retained-free render pipeline and an audio
interface — plus one SFML backend and one CPU-only (headless) backend that implement
those interfaces.

The engine itself links **no graphics or audio library**. Only the demo application
under `app/` knows SFML exists, which is what makes `-DSIF_BUILD_DEMO_APP=OFF` a
complete, testable build on a machine with no graphics stack at all.

---

## Layout

```
sif/            the engine (no SFML anywhere)
  include/sif/
    asset/      AssetRegistry, AssetRecord, AssetHandle, per-type asset interfaces
    audio/      AudioPlayer interface
    diagnostics/Logger, include-cycle analysis (PlantUML export)
    event/      Event_Bus, Observer, Event_Store, event definitions
    internal/   GUID, RecordID, Rect, Color, Size, Random, Delta_Timer
    io/         JSON helpers
    layout_engine/  Tokenizer -> Parser -> UIFactory -> UIElement tree
    math/       Vector, Matrix, Vector2, Point2, DirectedGraph
    render/     RenderFrame, RenderNode visitor tree, Renderer interface,
                Camera (normalized world [-1,1] -> pixels), RB_Config
  src/          implementations
  test/         self-registering test suite (no external framework)

app/            demo application (the only SFML consumer)
  sfml/         SFML backend: renderer, audio player, event collector, asset loaders
  headless/     CPU-only backend: decodes assets, draws nothing, opens no device
  demos/        four windowed demos + the headless verification run

data/           assets, asset descriptors, authoring scenes and serialized scenes
uml/            PlantUML diagrams of the subsystems
```

---

## Building

Requirements: CMake ≥ 3.20, a C++20 compiler, `nlohmann/json` at `external/json/json.hpp`,
and (for the demo application only) SFML 2.6 with the `audio` component.

Verified on the reference platform: **Ubuntu 24.04, GCC 13, SFML 2.6.1, CMake 3.28**.

```bash
# engine + tests only — no SFML required
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSIF_BUILD_DEMO_APP=OFF
cmake --build build -j
./build/sif/test/sif_tests

# everything, including the demo application
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The build is warning-clean under `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`.

---

## The asset pipeline

Assets are described by `*.asset.json` files that live next to the data they describe.
Two command-line tools turn those into what the runtime consumes:

```bash
# 1. scan data/, assign/collect GUIDs, write the registry
./build/sif/Asset_GUID_Assignment data/ data/bin/registry.rgst.json

# 2. rewrite authoring scenes (asset_name="...") into serialized ones (guid="...")
./build/sif/Asset_Reference_Serialization data/bin/registry.rgst.json data/scenes data/bin/scenes
```

Authoring files stay human-readable (`asset_name="DemoBall"`); the runtime only ever sees
stable numeric GUIDs, so renaming an asset cannot silently break a scene.

Supported asset types: `Font`, `SpriteSingle`, `SpriteAtlas`, `SpriteGrid`,
`PrimitiveAnimation`, `Sound`.

---

## Running the demos

From the build directory:

```bash
./app/sif_demo <demo-id> [data-dir]     # data-dir defaults to ../data/
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
./build/sif/test/sif_tests
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

`.circleci/config.yml` runs two jobs: one builds and tests the engine **without SFML
installed**, the other builds everything and runs the headless asset verification.
