# sif — feature audit

Audit of what the engine does not do yet, or does only partly, made after adding UI layers
and overlapping (see "UI layers and overlapping" in the README). Every entry points at the
code it is based on; entries marked **confirmed** were reproduced with a probe program, a
test or a screenshot, not only read off the source.

Status: every P1 defect and every P2 feature is done (sections 1 and 2, each with how it
was resolved and the commit that did it); section 3 lists what is still open (P3).

Priorities: **P1** — wrong behaviour today, or something a scene author hits immediately;
**P2** — a missing feature most games built on sif will need; **P3** — worthwhile, but
no one is blocked without it.

---

## 1. Defects found along the way (P1) — all fixed

Behaviour that contradicted the engine's own documentation. All seven are fixed, each with
its own tests (`LayoutFixTests.cpp`, `TokenizerTests.cpp`, `AssetRegistryTests.cpp`, the
UTF-8 section of the headless check); the table keeps the original finding and says how it
was resolved.

| # | Finding | Resolution |
|---|---|---|
| 1.1 | **`PERCENT` sizes were 100× too large**: `PERCENT\|50` of 800 px resolved to 40 000 px, in three separate copies of the resolver (`UIElement::resolve_size`, `Rectangle::resolve`, `Button::measure`). | `intrnl::Size::resolve()` is now the one definition (`PERCENT` is 0–100, as documented) and every element, `Overlay` and `LinearLayout` use it; `Rectangle::resolve` is gone. **Consumer note:** a scene that compensated for the bug (e.g. `PERCENT\|1` meaning "full width") must be updated. |
| 1.2 | **The tokenizer hung on an XML comment** and on any unexpected character inside a tag. | Comments, `<?xml ?>` prologs and `<!DOCTYPE>`-style declarations are skipped (a comment inside `<Text>` is dropped without cutting the text). Every malformed input is now a `std::runtime_error` naming the file, **line and column** (stray character, unterminated tag/value/comment, missing tag name, `/` without `>`); CDATA is rejected with a clear message. Valueless attributes are still accepted. |
| 1.3 | **Non-ASCII text rendered as one box per byte.** | The SFML backend decodes engine text as UTF-8 (`sfml::utf8_string`, used for drawing *and* measuring); the headless font measures characters instead of bytes; `asset::Font` documents UTF-8 as the contract. Verified by screenshot (Ukrainian text incl. Ґ/ї/є) and by the headless check. |
| 1.4 | **Serialized scenes broke on quotes**; entities were not decoded. | `&lt; &gt; &amp; &quot; &apos;` and `&#NNN;`/`&#xHH;` are decoded in attribute values and text; `save_tokens` escapes on write, so values survive `Asset_Reference_Serialization` unchanged. A bare `&` or an unknown entity is kept literally, so existing text keeps working. |
| 1.5 | **`LinearLayout` (VBox/HBox/Menu) ignored `padding`, `margin` and its own `width`/`height`.** | All three are applied (padding insets the content box, margins are fixed space around each child — flex shares what is left after them — and an explicit own size overrides the content size). Measured sizes are clamped to `min_size..max_size` like every other element, so a row can no longer report a negative width. A differential test over 400 random trees confirmed that layouts without padding/margin are unchanged except for that clamp. Demo scenes were reviewed with screenshots; only the `all` demo's tile strip needed `width="PERCENT\|100"` (see 2.12). |
| 1.6 | **A failed asset could never be retried.** | `AssetRegistry::retry(guid)`: re-queues a `Failed` asset like a first request; returns `false` in any other state. `request()` keeps ignoring failed assets, so per-frame callers cannot hammer a broken file. |
| 1.7 | **Typo `<0VBox` in the working copy of `data/scenes/demo_all.ui.xml`.** | Reverted. |

---

## 2. P2 features — all implemented (2.0.5)

One commit per item on `sif-au`, each with its own tests (`sif_tests`, 175 cases at the end
of the round) and, where it shows on screen, a check in the headless run and a demo.

| # | Feature | How it was resolved | Commit |
|---|---|---|---|
| 2.1 | **Clipping and ScrollView** | `RenderNode::clip`, stamped by `RenderFrame::submit` from an RAII `rnd::ClipScope` (nested clips intersect); SFML scissors through a clip-sized `sf::View`. `clip="true"` on any element; hit-testing honours clips. `<ScrollView direction scroll_step follow_end>` offers its content unlimited space along the scroll axis and shows it through a clipped viewport; wheel scrolling bubbles on at either end. | `ff4b6c8` |
| 2.2 | **Input routing into the tree** | `ui::InputRouter` (Mediator) turns raw input into `ui::UIEvent`s bubbled along `hit_path` (Chain of Responsibility; *handled* and *stop propagation* are separate). `hovered`/`pressed`/`focused` on elements; Click on press+release over the same element; focus on click, arrows (while something is focused), Tab/Shift+Tab; Enter/Space activate. `Button::on_click` (Command), `enabled`, hover/press/disabled/focus visuals; `Menu` follows clicks and focus. `App` routes input to the UI first. Demo `widgets`. | `7813f63` |
| 2.3 | **justify** | `justify="Start\|Center\|End\|SpaceBetween\|SpaceAround\|SpaceEvenly"` on VBox/HBox/Menu; no effect while a flex child takes the space. | `72df2d3` |
| 2.4 | **Text layout** | `h_text_align`/`v_text_align`, `wrap` (`\n` = hard break, indentation after it dropped), `max_lines`, `ellipsis` (per UTF-8 character), `line_spacing`, `stretch_font` from XML; one `rnd::Text` per line, lines in a lazy cache so text changed after layout still renders that frame. `Text` now honours an explicit `width`/`height` (it ignored them). | `6d43d4c` |
| 2.5 | **Styles and included scenes** | `ui::SceneComposer` between Parser and UIFactory: `<Include src>` (relative, nested, cycle-checked, overridable), `<Style name>` + `class="a b"` (later class wins, own attributes win over all). Nodes remember their file, so errors read `file, line, column`. `ui::SceneLoader::load` runs the whole pipeline. | `4c17f31` |
| 2.6 | **Scene validation** | Every built-in tag registered with its attribute schema; an unknown attribute is a logged warning with position and a "did you mean" suggestion, a `SceneError` under `UIFactory::set_strict(true)`; build errors carry the element's position once. The headless run builds every demo scene in strict mode. | `8394969` |
| 2.12 | **Rows over-measuring** | Not as first proposed (measuring flex children at zero would have broken "Auto fills its slot"): a layout's main-axis content size is capped at what it was offered; overflowing children still overflow inside it. The `all` demo no longer needs its workaround. | `39cc987` |
| 3.1 | **World rendering through Camera** | `<WorldView aspect zoom center y_axis_up>` projects each child's `world_rect` through its `rnd::Camera`, culls what is out of view, clips to its rect, maps clicks back with `to_world()`. Demo `world`. | `af3ab28` |
| 3.2 | **Headless renderer** | `rnd::RecordingRenderer` (records every draw call in draw order), `event::Scripted_Event_Collector`, `audio::Null_AudioPlayer` - engine-side, no SFML, built by `Graphics_Factory` for `RB_Type::Headless`. `App::run(max_frames)`; the headless run drives every demo through `App` for 30 frames. | `e520164` |
| 4.1 | **Unloading** | `AssetRegistry::release(guid)` (safe while in use: data is shared; queued/loading assets handled through the queues and the attempt guard), asset groups (`add_to_group`, `request_group`, `release_group` that keeps assets another active group needs) and the RAII `GroupCapture` that turns a scene load into a group; `state_of`, `loaded_count`. | `749f6bb` |
| 5.1 | **Audio control and buses** | `play(sound, PlayOptions)` (volume, loop, bus, pitch, pan, fade-in), per-voice volume/pitch/pan/pause/resume/fades, Effects/Music/Interface buses, `update(dt)` from App; loudness is voice × bus × master everywhere (fixes `set_master_volume` overwriting voice volumes). New interface methods are pure virtual. | `4e01a3c` |
| 6.1 | **Key coverage** | 98 keys from one X-macro list (`SIF_KEYS`) that also generates `key_name()` and the SFML mapping; the original 15 keep their values. | `b0af311` |

The round also fixed the repository's line-ending storage (`.gitattributes`, `7c0494a`) and
added two demos (`widgets`, `world`).

---

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

## Suggested order

1. ~~The P1 defects (section 1)~~ — done (2.0.3/2.0.4).
2. ~~The P2 features (section 2)~~ — done (2.0.5).
3. Of the P3 items, incremental layout (dirty flags) and inherited opacity give the most
   for a game UI; a scene linter (7.3) is now a thin wrapper around `SceneLoader` in strict
   mode.
