You are a senior game engine programmer working in C.

Your task is to design and implement a modular raycasting-based game engine inspired by classic engines like Doom, but with a modern architecture similar to Godot.

IMPORTANT CONTEXT:
- This project is based on a completed cub3D (42 school project).
- You are allowed to fully refactor and expand it.
- The goal is to transform it into a DATA-DRIVEN + SCRIPTABLE ENGINE.
- The engine must be GENERIC and NOT tied to any specific game (no zombies, no fixed gameplay).

--------------------------------------------------
CORE GOALS
--------------------------------------------------

1. The engine MUST be divided into 3 layers:

(1) CORE ENGINE (C)
- Raycasting renderer
- World/map system
- Physics and collision
- Entity system
- Audio system
- Resource management

(2) SCRIPTING LAYER (Lua)
- Gameplay logic must be written in Lua
- No hardcoded gameplay in C
- C only provides APIs

(3) EDITOR (cimgui + MLX42)
- GUI similar to a simplified Godot editor
- Real-time editing
- Preview system (2D + 3D)
- Inspector panel
- Asset browser
- File system panel
- Console / build logs

--------------------------------------------------
DATA-DRIVEN ARCHITECTURE
--------------------------------------------------

- The engine must NOT depend on .cub files anymore.
- Use JSON (via cJSON) for all configurations:
  - maps
  - entities
  - prefabs
  - audio
  - project settings

- The editor modifies JSON files.
- The engine loads JSON files and builds runtime structures.

Example:
- maps/level01.json
- prefabs/enemy.json
- scripts/enemy.lua

--------------------------------------------------
ENTITY SYSTEM (NODE-BASED)
--------------------------------------------------

Implement a Node/Entity system inspired by scene trees:

- Each entity has:
  - unique ID
  - type (string)
  - transform (x, y, rotation)
  - parent/children
  - script reference
  - properties (dynamic)

- Maintain:
  - global entity table (O(1) access)
  - scene root node

--------------------------------------------------
PREFAB SYSTEM
--------------------------------------------------

- Prefabs are defined in JSON.
- Example:
  {
    "name": "enemy_basic",
    "script": "enemy.lua",
    "properties": {
      "hp": 100,
      "speed": 2.0
    }
  }

- Engine must support:
  instantiate(prefab_name, x, y)

--------------------------------------------------
LUA SCRIPTING SYSTEM
--------------------------------------------------

Integrate Lua (5.4).

Implement:
- lua_engine_init()
- lua bindings
- script loader

Expose API modules:

engine.entity
engine.world
engine.player
engine.input
engine.audio
engine.ui

Example Lua usage:

function update(dt)
    engine.entity.move(id, 1 * dt, 0)
end

--------------------------------------------------
LIFECYCLE SYSTEM (IMPORTANT)
--------------------------------------------------

Each script may define:

init(id)
ready(id)
update(id, dt)
destroy(id)

Engine must call these automatically.

--------------------------------------------------
INSPECTOR SYSTEM (EDITOR)
--------------------------------------------------

Support editable variables from Lua:

Example:

Enemy.export = {
    speed = 2.0,
    hp = 100
}

- Editor must read these values
- Allow editing per instance
- Store overrides in JSON

--------------------------------------------------
REAL-TIME PREVIEW
--------------------------------------------------

- Use MLX42 for rendering
- Use framebuffer for 3D preview
- Display inside cimgui window
- Changes in editor MUST reflect instantly

--------------------------------------------------
EDITOR FEATURES (cimgui)
--------------------------------------------------

Implement panels:

- Scene View (2D map editor)
- 3D Preview viewport
- Inspector (entity properties)
- Asset Browser
- File System tree
- Console / Logs

Buttons:
- Run game
- Save project
- Reload scripts (hot reload)

--------------------------------------------------
HOT RELOAD SYSTEM
--------------------------------------------------

- Reload Lua scripts at runtime
- Reset Lua state safely
- Rebind APIs

--------------------------------------------------
AUDIO SYSTEM
--------------------------------------------------

- Load sounds from config
- Play via events
- Allow Lua to trigger sounds

--------------------------------------------------
HUD SYSTEM (SCRIPTED)
--------------------------------------------------

Lua must be able to draw UI:

ui.draw_text()
ui.draw_sprite()

HUD is updated via Lua scripts.

--------------------------------------------------
DELTA TIME
--------------------------------------------------

All update functions must receive dt (delta time).

--------------------------------------------------
EXPORT SYSTEM
--------------------------------------------------

- Provide a "Build/Export" feature in editor
- Generate executable for user's OS
- Bundle:
  - engine runtime
  - assets
  - scripts

User should NOT need to run via terminal.

--------------------------------------------------
CODE ORGANIZATION
--------------------------------------------------

Structure:

engine/
  core/
  scripting/
  scene/
  renderer/
  audio/

editor/
  gui/
  panels/

assets/
scripts/
prefabs/
maps/

--------------------------------------------------
IMPORTANT RULES
--------------------------------------------------

- DO NOT hardcode gameplay
- EVERYTHING must be data-driven or scripted
- Keep C focused on performance and systems
- Keep Lua focused on gameplay
- Keep editor independent from runtime logic

--------------------------------------------------
OUTPUT EXPECTATION
--------------------------------------------------

Start by implementing:

1. Core entity system (node + ID)
2. Prefab loader (JSON)
3. Lua integration (init + update loop)
4. Basic API bindings (move entity)
5. Minimal editor window (cimgui)

Write clean, modular, production-quality C code.
