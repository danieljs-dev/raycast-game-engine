# Raycast Game Engine — API Reference (WIP)

This document is a “Godot-like” reference for the APIs and data formats that are implemented **right now** in this repository.

## 1) Concepts

### 1.1 Project (JSON)
A **project** is a JSON file (typically under `maps/`) that defines:
- Textures
- Floor/Ceiling colors
- The map grid (including player spawn marker)
- Prefab directory + which prefab to use as player
- Optional scene instances file
- Optional player overrides

At runtime/editor, the project is what you load/save.

### 1.2 Prefab (JSON)
A **prefab** is a JSON file (typically under `prefabs/`) that defines:
- A prefab `name`
- The entity `type`
- Optional Lua script path
- Default exported properties

Prefabs are used to instantiate entities for:
- The **player** (always)
- Any entities in a **scene instances** file

### 1.3 Scene Instances (JSON)
A **scene instances** file (typically under `scenes/`) defines a list of entities to instantiate from prefabs with transforms + property overrides.

### 1.4 Entity Properties & `export`
Each entity has a property bag (number/bool/string) used by:
- Prefab defaults
- Inspector editing
- Lua scripting via an `export` table

The engine keeps Lua `export` and the entity property bag synchronized (details below).

## 2) Directory Layout (expected)

- `assets/` textures and other assets
- `scripts/` `.lua` scripts
- `prefabs/` prefab JSON files
- `maps/` project JSON files
- `scenes/` scene instance JSON files

## 3) Data Formats

### 3.1 Project JSON (maps/*.json)

Minimal shape:

```json
{
  "textures": { "NO": "...", "SO": "...", "WE": "...", "EA": "..." },
  "colors": {
    "floor": [r, g, b],
    "ceiling": [r, g, b]
  },
  "map": [
    "111111",
    "10N001",
    "111111"
  ]
}
```

Optional keys currently used:
- `"prefabs_dir"`: string. Defaults to `"prefabs"`.
- `"player_prefab"`: string. Defaults to `"player"`.
- `"scene"`: string path to a scene instances file, e.g. `"scenes/level01.scene.json"`.
- `"player_overrides"`: object with key/value pairs (number/bool/string) applied on top of the player entity’s properties after creation.

Notes:
- The player spawn is part of the map data: one of `N`, `S`, `E`, `W` must appear on the grid.
- Saving the project re-inserts the current spawn marker back into the serialized map.

### 3.2 Scene Instances JSON (scenes/*.scene.json)

Shape:

```json
{
  "entities": [
    {
      "type": "enemy",
      "x": 3.5,
      "y": 6.0,
      "rot": 0.0,
      "properties": {
        "hp": 100,
        "aggressive": true,
        "name": "Bob"
      }
    }
  ]
}
```

Supported keys per entity:
- `type` (string): prefab name to instantiate.
- `prefab` (string): also supported as an alias; if present it is preferred.
- `x`, `y` (number): entity position.
- `rot` (number): entity rotation in radians.
- `properties` (object): property overrides (number/bool/string).

Notes:
- Entities with `type == "root"` are ignored on load and not saved.
- Scene saving skips the player entity by id (so you don’t duplicate player instances).

### 3.3 Prefab JSON (prefabs/*.json)

Shape:

```json
{
  "name": "player",
  "type": "player",
  "script": "scripts/player.lua",
  "properties": {
    "speed": 3.0,
    "rot_speed": 2.5,
    "debug_hud": true
  }
}
```

Notes:
- `script` is optional.
- `properties` is optional; values may be number/bool/string.
- Internally, the prefab system stores `type` as a reserved property `__type`.

## 4) Lua Scripting

### 4.1 Script Environment
Each entity script runs in its own environment table with:
- `engine` injected (same global engine API table for all scripts)
- access to standard Lua libraries

### 4.2 Lifecycle Functions
If present, these functions are called by the engine:

```lua
function init(id) end
function ready(id) end
function update(id, dt) end
function destroy(id) end
```

Call order:
- On entity script load: `init(id)` then `ready(id)`
- Every tick: `update(id, dt)`
- On reload/unload: scripts are unloaded and reloaded; lifecycle runs again

### 4.3 `export` table (Inspector-exposed properties)
Scripts may define a global `export` table:

```lua
export = {
  speed = 3.0,
  debug_hud = true
}
```

Behavior:
- On script load, the engine copies `export` keys into the entity property bag **if the property does not already exist**.
- On every lifecycle call (`init/ready/update`), the engine syncs entity properties **back into** `export` (only for keys that already exist in `export`).

Reserved keys:
- Any property key starting with `__` is treated as internal/reserved and ignored by the export/property sync.

### 4.4 Global `print()`
Lua `print(...)` is overridden by the engine. In the editor, it is routed to the Editor Console.

## 5) Lua API: `engine.*`

All APIs below are currently implemented in the `engine` global table.

### 5.1 `engine.entity`

#### `engine.entity.move(id, dx, dy)`
Moves an entity by `(dx, dy)` in world coordinates.

Notes:
- If `id` is the current player, movement is collision-aware against the map (`'1'` walls) using a small radius.
- Other entities currently move without collision.

#### `engine.entity.move_local(id, forward, strafe)`
Moves an entity relative to its rotation.

- `forward`: forward/back movement magnitude (world units)
- `strafe`: left/right movement magnitude (world units)

Collision notes are the same as `move()`.

#### `engine.entity.rotate(id, drot)`
Rotates an entity by `drot` radians.

### 5.2 `engine.world`

#### `engine.world.is_wall(x, y) -> boolean`
Returns true if the map cell at integer grid coordinates `(x, y)` is a wall (`'1'`).

Out-of-range behavior:
- If `(x, y)` is outside the map bounds (or line is shorter), it returns `true`.

### 5.3 `engine.player`

#### `engine.player.id() -> integer`
Returns the current player entity id.

### 5.4 `engine.input`

#### `engine.input.is_key_down(key) -> boolean`
Returns whether the given key code is currently pressed.

Notes:
- Key codes are MLX42 `keys_t` values (GLFW-like). Example usage is in `scripts/player.lua`.

### 5.5 `engine.ui`

#### `engine.ui.draw_text(x, y, text)`
Draws text on screen using MLX42.

Notes:
- Text draw calls allocate MLX images that are automatically deleted by the Lua engine at the start of the next frame.
- This makes it suitable for “immediate mode” HUD usage.

### 5.6 `engine.audio`

#### `engine.audio.play(name)`
Currently a stub (no output).

## 6) Editor Behaviors Relevant to Scripting

### 6.1 Toolbar
- `Save`: saves the project JSON and the scene instances JSON (if a scene path exists; the editor auto-creates a default scene path on first save).
- `Reload scripts`: rebuilds the Lua VM and reloads scripts for all entities; re-runs `init/ready`.
- `Build/Export`: builds and stages a `dist/` folder.

### 6.2 Console
- Receives Lua `print(...)` and Lua errors raised by `luaL_loadfile` / `pcall`.

### 6.3 Script Editor panel
- Lists `scripts/*.lua`.
- `Save`: writes the buffer back to disk.
- `Reload`: reloads the file from disk.

(Currently it does not automatically call “Reload scripts” after saving; you can do it from the toolbar.)

## 7) Current Limitations (intentional / not implemented yet)

- No general collision/physics system for non-player entities.
- No Lua API to instantiate prefabs/entities at runtime.
- `engine.audio` is stub.
- UI only supports text drawing (`draw_sprite` not implemented).
- Scene hierarchy is not a full tree yet (no children arrays / no editor parenting UI).

---

If you want, I can also generate a second doc page focused only on **Editor UX shortcuts** (mouse/keys, panel responsibilities) once we lock the final panel names and workflow.
