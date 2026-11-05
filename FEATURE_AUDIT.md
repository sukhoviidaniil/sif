# sif — feature audit

## 3. Still open (P3)

### UI layout engine

| # | Pri | Missing / partial | Why it matters | Suggested approach |
|---|---|---|---|---|
| 2.7 | P3 | **Inherited opacity** (`opacity="0.5"` on a subtree) | Fading a dialog or a whole HUD in/out; today only per-item colours can be changed. | Same RAII scope technique as `LayerScope`; multiply into colours/tints at `submit()`. |
| 2.8 | P3 | **Grid** and **wrapping** containers | Inventories, level-select screens, keyboard layouts. | New `LinearLayout` siblings; rows/cols as attributes. |
| 2.9 | P3 | **Rectangle/Sprite styling from XML**: `border_color`/`border_width` (fields exist, not parsed), rounded corners, aspect-preserving `fit="Contain\|Cover"` for Sprite, flip | Sprites are always stretched to their rect; borders need C++. | Parse the existing fields; add a `fit` mode resolved into the destination rect inside `ui::Sprite`. |
| 2.10 | P3 | **Nine-slice sprites** | Scalable panels and buttons from one small texture. | New asset type (or SpriteAtlas metadata) + element emitting nine `rnd::Sprite`s. |
| 2.11 | P3 | **Incremental layout** (dirty flags) | Measure runs every frame and repeatedly: App measures the root, then every `LinearLayout::layout` measures each child twice — O(n·depth) per frame. `RenderFrame::constant_items` and `FrameContext::redrawing` were clearly meant for caching but nothing uses them. (`ui::Text` now caches its line breaking, the most expensive per-frame work it did.) | `invalidate()` on property change propagating to the root; skip measure/layout of clean subtrees; keep their render items in `constant_items`. |

### Render pipeline

| # | Pri | Missing / partial | Why it matters | Suggested approach |
|---|---|---|---|---|
| 3.3 | P3 | **More primitives and transforms**: lines, circles/polygons, rotation/scale/origin, blend modes | Only Rectangle, Text and Sprite exist; nothing can rotate. | New `RenderNode` subtypes (the Visitor already makes the cost explicit: one `visit()` per backend). A transform on `RenderNode` covers rotation/scale for all of them. |
| 3.4 | P3 | **Window options**: fullscreen, vsync, resizable/fixed, icon | `RB_Config` has only name, size and fps. | Extra fields consumed by `SFML_Renderer`'s constructor. |

### Asset system

| # | Pri | Missing / partial | Why it matters | Suggested approach |
|---|---|---|---|---|
| 4.2 | P3 | **Hot reload** during development | Editing a texture or a scene requires a restart. | Poll `last_write_time` of loaded sources in `pump()` (debug builds); reload through the existing attempt-token mechanism, which already discards stale loads. |
| 4.3 | P3 | **Loading progress** | `AssetRegistry::loaded_count()` and `state_of()` exist now (4.1), but there is no "n of m in this group are ready" query for a loading screen. | Counters per state in `AssetRegistry`, updated where states already change. |
| 4.4 | P3 | **More asset types**: streamed music, shaders, tile maps, string tables (localisation) | Music is decoded fully into an `sf::SoundBuffer`; there is no way to stream a long track. | New `AssetType`s + loaders; `sf::Music` for streaming. |

### Input

| # | Pri | Missing / partial | Why it matters | Suggested approach |
|---|---|---|---|---|
| 6.2 | P3 | **Gamepad / joystick** | Not represented at all. | New event structs + collector cases, same pattern as mouse. |

### Tooling, tests, CI

| # | Pri | Missing / partial | Why it matters | Suggested approach |
|---|---|---|---|---|
| 7.1 | P3 | **Deterministic serialized scenes** | `Token::attributes` is an `unordered_map`, so `Asset_Reference_Serialization` writes attributes in arbitrary order (they come out reversed for `demo_layers`) — noisy diffs, and the output can change between standard-library versions. | Keep attributes as an ordered `vector<pair>` in `Token`/`Node`. |
| 7.2 | P3 | **Tests write into the working directory** | `UIElementTests`, `LayerTests`, `TokenizerTests` and a few newer tests write their sample `*.ui.xml` into the CWD; a copy, `sif_test_sample.ui.xml`, is committed at the repository root. | Write under `std::filesystem::temp_directory_path()` and remove the committed file. |
| 7.3 | P3 | **A scene linter / previewer** | A malformed or misspelled scene is only discovered when a game loads it. | A command-line wrapper around `ui::SceneLoader::load` in strict mode with the headless backend's loaders - the headless run already does exactly this for the demo scenes (2.6). |

---

