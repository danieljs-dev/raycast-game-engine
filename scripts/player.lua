export = {
    speed = 3.0,
    rot_speed = 2.5,
    sprint_mul = 1.75,
    debug_hud = true,
    debug_print = false,
    debug_print_hz = 4.0,

    -- Spawn example (Lua): press O to spawn one entity
    debug_spawn = false,
    debug_spawn_prefab = "enemy",
    debug_spawn_x = 3.5,
    debug_spawn_y = 6.0
}

function init(id)
    print("player.init id=", id)
end

function ready(id)
    print("player.ready id=", id)
end

local __acc = 0.0
local __print_acc = 0.0
local __spawn_was_down = false

function update(id, dt)
    __acc = __acc + dt
    __print_acc = __print_acc + dt

    local speed = export.speed
    if engine.input.is_key_down(340) then -- MLX_KEY_LEFT_SHIFT
        speed = speed * export.sprint_mul
    end

    -- Movement: local-space (forward/strafe), collision is handled by engine for player
    local forward = 0.0
    local strafe = 0.0
    if engine.input.is_key_down(87) then -- MLX_KEY_W
        forward = forward + 1.0
    end
    if engine.input.is_key_down(83) then -- MLX_KEY_S
        forward = forward - 1.0
    end
    if engine.input.is_key_down(68) then -- MLX_KEY_D
        strafe = strafe + 1.0
    end
    if engine.input.is_key_down(65) then -- MLX_KEY_A
        strafe = strafe - 1.0
    end
    if forward ~= 0.0 or strafe ~= 0.0 then
        engine.entity.move_local(id, forward * speed * dt, strafe * speed * dt)
    end

    -- Rotation
    if engine.input.is_key_down(263) then -- MLX_KEY_LEFT
        engine.entity.rotate(id, -export.rot_speed * dt)
    end
    if engine.input.is_key_down(262) then -- MLX_KEY_RIGHT
        engine.entity.rotate(id, export.rot_speed * dt)
    end

    -- Audio example (stub for now): press P
    if engine.input.is_key_down(80) then -- MLX_KEY_P
        engine.audio.play("test")
    end

    -- Instantiate example: press O once (edge-triggered)
    local spawn_down = engine.input.is_key_down(79) -- MLX_KEY_O
    if export.debug_spawn and spawn_down and (not __spawn_was_down) then
        local new_id = engine.entity.instantiate(export.debug_spawn_prefab, export.debug_spawn_x, export.debug_spawn_y)
        print("spawn", export.debug_spawn_prefab, "id=", new_id)
    end
    __spawn_was_down = spawn_down

    -- World query example
    local wall_ahead = engine.world.is_wall(1, 1)

    -- HUD / UI example
    if export.debug_hud then
        engine.ui.draw_text(100, 16, string.format("dt=%.4f", dt))
        engine.ui.draw_text(100, 32, string.format("speed=%.2f rot=%.2f", export.speed, export.rot_speed))
        engine.ui.draw_text(100, 48, string.format("wall(1,1)=%s", wall_ahead and "true" or "false"))
        engine.ui.draw_text(100, 64, string.format("player_id=%d", engine.player.id()))
    end

    -- Print example (goes to Editor Console now)
    if export.debug_print and export.debug_print_hz > 0.0 then
        local period = 1.0 / export.debug_print_hz
        if __print_acc >= period then
            __print_acc = __print_acc - period
            print("tick", string.format("t=%.2f", __acc), "speed", export.speed)
        end
    end
end

function destroy(id)
    print("player.destroy id=", id)
end
