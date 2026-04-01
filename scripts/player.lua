export = {
    speed = 3.0,
    rot_speed = 2.5
}

function init(id)
end

function ready(id)
end

function update(id, dt)
    local speed = export.speed
    if engine.input.is_key_down(340) then -- MLX_KEY_LEFT_SHIFT
        speed = speed * 1.75
    end
    if engine.input.is_key_down(87) then -- MLX_KEY_W
        engine.entity.move(id, speed * dt, 0.0)
    end
    if engine.input.is_key_down(83) then -- MLX_KEY_S
        engine.entity.move(id, -speed * dt, 0.0)
    end
    if engine.input.is_key_down(65) then -- MLX_KEY_A
        engine.entity.move(id, 0.0, -speed * dt)
    end
    if engine.input.is_key_down(68) then -- MLX_KEY_D
        engine.entity.move(id, 0.0, speed * dt)
    end
    if engine.input.is_key_down(263) then -- MLX_KEY_LEFT
        engine.entity.rotate(id, -export.rot_speed * dt)
    end
    if engine.input.is_key_down(262) then -- MLX_KEY_RIGHT
        engine.entity.rotate(id, export.rot_speed * dt)
    end
end

function destroy(id)
end
