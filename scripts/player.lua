function init(id)
end

function ready(id)
end

function update(id, dt)
    engine.entity.move(id, 0.75 * dt, 0.0)
end

function destroy(id)
end
