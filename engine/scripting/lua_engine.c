/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lua_engine.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/scripting/lua_engine.h"

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

#include <MLX42/MLX42.h>

#include "engine/scene/entity.h"
#include "engine/scene/prefab.h"
#include "structs.h"
#include "libft.h"
#include "config.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static void	lua_log_ex(t_lua_engine *lua, const char *msg)
{
	if (!msg || msg[0] == '\0')
		return ;
	if (lua && lua->log_fn)
		return (lua->log_fn(lua->log_user, msg));
	write(2, msg, ft_strlen(msg));
	write(2, "\n", 1);
}

static int	l_print(lua_State *L)
{
	t_lua_engine	*lua;
	int			n;
	int			i;
	const char		*s;
	char			buf[1024];
	int			off;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	n = lua_gettop(L);
	off = 0;
	i = 1;
	while (i <= n)
	{
		s = lua_tostring(L, i);
		if (!s)
			s = luaL_tolstring(L, i, NULL);
		if (s)
		{
			off += snprintf(buf + off, sizeof(buf) - (size_t)off,
				"%s%s", (i > 1) ? "\t" : "", s);
			if (off >= (int)sizeof(buf) - 1)
				break ;
		}
		if (lua_type(L, i) != LUA_TSTRING)
			lua_pop(L, 1);
		i++;
	}
	buf[sizeof(buf) - 1] = '\0';
	lua_log_ex(lua, buf);
	return (0);
}

static void	lua_entity_move_impl(t_lua_engine *lua, uint32_t id, t_entity *e,
		double dx, double dy)
{
	if (!lua || !e)
		return ;
	if (lua->world && id == lua->player_id)
	{
		double	nx;
		double	ny;
		double	r;
		int		mx;
		int		my;
		int		blocked;

		r = PLAYER_COLLISION_RADIUS;

		blocked = 0;
		nx = e->transform.x + dx;
		ny = e->transform.y;
		mx = (int)(nx + (dx > 0.0 ? r : -r));
		my = (int)(ny);
		if (my < 0 || my >= lua->world->map_height
			|| mx < 0 || mx >= lua->world->map_width)
			blocked = 1;
		else if (lua->world->map[my]
			&& mx < (int)ft_strlen(lua->world->map[my])
			&& lua->world->map[my][mx] == '1')
			blocked = 1;
		if (!blocked)
			e->transform.x = nx;

		blocked = 0;
		nx = e->transform.x;
		ny = e->transform.y + dy;
		mx = (int)(nx);
		my = (int)(ny + (dy > 0.0 ? r : -r));
		if (my < 0 || my >= lua->world->map_height
			|| mx < 0 || mx >= lua->world->map_width)
			blocked = 1;
		else if (lua->world->map[my]
			&& mx < (int)ft_strlen(lua->world->map[my])
			&& lua->world->map[my][mx] == '1')
			blocked = 1;
		if (!blocked)
			e->transform.y = ny;
		return ;
	}
	entity_move(e, dx, dy);
}

static int	l_entity_move(lua_State *L)
{
	t_lua_engine	*lua;
	uint32_t		id;
	double			dx;
	double			dy;
	t_entity		*e;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->store)
		return (0);
	id = (uint32_t)luaL_checkinteger(L, 1);
	dx = (double)luaL_checknumber(L, 2);
	dy = (double)luaL_checknumber(L, 3);
	e = entity_get(lua->store, id);
	if (e)
		lua_entity_move_impl(lua, id, e, dx, dy);
	return (0);
}

static int	l_entity_move_local(lua_State *L)
{
	t_lua_engine	*lua;
	uint32_t		id;
	double			forward;
	double			strafe;
	t_entity		*e;
	double			cs;
	double			sn;
	double			dx;
	double			dy;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->store)
		return (0);
	id = (uint32_t)luaL_checkinteger(L, 1);
	forward = (double)luaL_checknumber(L, 2);
	strafe = (double)luaL_checknumber(L, 3);
	e = entity_get(lua->store, id);
	if (!e)
		return (0);
	cs = cos(e->transform.rot);
	sn = sin(e->transform.rot);
	dx = (forward * cs) + (strafe * -sn);
	dy = (forward * sn) + (strafe * cs);
	lua_entity_move_impl(lua, id, e, dx, dy);
	return (0);
}

static int	l_entity_rotate(lua_State *L)
{
	t_lua_engine	*lua;
	uint32_t		id;
	double			drot;
	t_entity		*e;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->store)
		return (0);
	id = (uint32_t)luaL_checkinteger(L, 1);
	drot = (double)luaL_checknumber(L, 2);
	e = entity_get(lua->store, id);
	if (e)
		entity_rotate(e, drot);
	return (0);
}

uint32_t	lua_engine_entity_instantiate(t_lua_engine *lua,
			const char *prefab_name, double x, double y)
{
	t_entity		*e;
	t_lua_script	*s;

	if (!lua || !lua->store || !lua->prefabs || !prefab_name || prefab_name[0] == '\0')
		return (0);
	e = prefab_instantiate(lua->prefabs, lua->store, prefab_name, x, y);
	if (!e)
		return (0);
	if (e->script_path)
	{
		s = ft_calloc(1, sizeof(*s));
		if (!s)
			return (entity_destroy(lua->store, e->id), 0);
		if (!lua_script_load(lua, s, e->script_path))
			return (free(s), entity_destroy(lua->store, e->id), 0);
		e->script = s;
		lua_script_apply_exports(lua, s, e, 0);
		lua_call_init(lua, s, e->id);
		lua_call_ready(lua, s, e->id);
	}
	return (e->id);
}

static int	l_entity_instantiate(lua_State *L)
{
	t_lua_engine	*lua;
	const char		*prefab;
	double			x;
	double			y;
	uint32_t		id;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua)
		return (lua_pushinteger(L, 0), 1);
	prefab = luaL_checkstring(L, 1);
	x = (double)luaL_checknumber(L, 2);
	y = (double)luaL_checknumber(L, 3);
	id = lua_engine_entity_instantiate(lua, prefab, x, y);
	lua_pushinteger(L, (lua_Integer)id);
	return (1);
}

static int	l_world_is_wall(lua_State *L)
{
	t_lua_engine	*lua;
	int			x;
	int			y;
	char			c;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->world || !lua->world->map)
		return (lua_pushboolean(L, 1), 1);
	x = (int)luaL_checkinteger(L, 1);
	y = (int)luaL_checkinteger(L, 2);
	if (x < 0 || y < 0 || y >= lua->world->map_height || x >= lua->world->map_width)
		return (lua_pushboolean(L, 1), 1);
	if (!lua->world->map[y])
		return (lua_pushboolean(L, 1), 1);
	if (x >= (int)ft_strlen(lua->world->map[y]))
		return (lua_pushboolean(L, 1), 1);
	c = lua->world->map[y][x];
	return (lua_pushboolean(L, c == '1'), 1);
}

static int	l_player_id(lua_State *L)
{
	t_lua_engine	*lua;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua)
		return (lua_pushinteger(L, 0), 1);
	return (lua_pushinteger(L, (lua_Integer)lua->player_id), 1);
}

static int	l_input_is_key_down(lua_State *L)
{
	t_lua_engine	*lua;
	int			key;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->mlx)
		return (lua_pushboolean(L, 0), 1);
	key = (int)luaL_checkinteger(L, 1);
	return (lua_pushboolean(L, mlx_is_key_down(lua->mlx, (keys_t)key)), 1);
}

static int	l_audio_play(lua_State *L)
{
	(void)L;
	return (0);
}

static int	ui_track_image(t_lua_engine *lua, void *img)
{
	void	**n;
	int		cap;
	int		i;

	if (!lua || !img)
		return (0);
	if (lua->ui_count + 1 <= lua->ui_cap)
	{
		lua->ui_images[lua->ui_count++] = img;
		return (1);
	}
	cap = lua->ui_cap;
	if (cap < 8)
		cap = 8;
	while (cap < lua->ui_count + 1)
		cap *= 2;
	n = (void **)ft_calloc((size_t)cap, sizeof(*n));
	if (!n)
		return (0);
	i = 0;
	while (i < lua->ui_count)
	{
		n[i] = lua->ui_images[i];
		i++;
	}
	free(lua->ui_images);
	lua->ui_images = n;
	lua->ui_cap = cap;
	lua->ui_images[lua->ui_count++] = img;
	return (1);
}

static int	l_ui_draw_text(lua_State *L)
{
	t_lua_engine	*lua;
	int			x;
	int			y;
	const char		*text;
	mlx_image_t		*img;

	lua = (t_lua_engine *)lua_touserdata(L, lua_upvalueindex(1));
	if (!lua || !lua->mlx)
		return (0);
	x = (int)luaL_checkinteger(L, 1);
	y = (int)luaL_checkinteger(L, 2);
	text = luaL_checkstring(L, 3);
	img = mlx_put_string(lua->mlx, text, x, y);
	if (img)
		ui_track_image(lua, img);
	return (0);
}

static void	register_engine_api(t_lua_engine *lua)
{
	lua_State	*L;

	L = (lua_State *)lua->L;
	lua_newtable(L);

	/* engine.entity */
	lua_newtable(L);
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_entity_move, 1);
	lua_setfield(L, -2, "move");
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_entity_move_local, 1);
	lua_setfield(L, -2, "move_local");
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_entity_rotate, 1);
	lua_setfield(L, -2, "rotate");
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_entity_instantiate, 1);
	lua_setfield(L, -2, "instantiate");
	lua_setfield(L, -2, "entity");

	/* engine.world */
	lua_newtable(L);
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_world_is_wall, 1);
	lua_setfield(L, -2, "is_wall");
	lua_setfield(L, -2, "world");

	/* engine.player */
	lua_newtable(L);
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_player_id, 1);
	lua_setfield(L, -2, "id");
	lua_setfield(L, -2, "player");

	/* engine.input */
	lua_newtable(L);
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_input_is_key_down, 1);
	lua_setfield(L, -2, "is_key_down");
	lua_setfield(L, -2, "input");

	/* engine.audio */
	lua_newtable(L);
	lua_pushcfunction(L, l_audio_play);
	lua_setfield(L, -2, "play");
	lua_setfield(L, -2, "audio");

	/* engine.ui */
	lua_newtable(L);
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_ui_draw_text, 1);
	lua_setfield(L, -2, "draw_text");
	lua_setfield(L, -2, "ui");

	lua_setglobal(L, "engine");

	/* Override global print() to go through engine logger (editor console). */
	lua_pushlightuserdata(L, lua);
	lua_pushcclosure(L, l_print, 1);
	lua_setglobal(L, "print");
}

int	lua_engine_init(t_lua_engine *lua, t_entity_store *store)
{
	lua_State	*L;

	if (!lua)
		return (0);
	ft_bzero(lua, sizeof(*lua));
	L = luaL_newstate();
	if (!L)
		return (0);
	luaL_openlibs(L);
	lua->L = (void *)L;
	lua->store = store;
	lua->prefabs = NULL;
	lua->mlx = NULL;
	lua->world = NULL;
	lua->player_id = 0;
	lua->log_fn = NULL;
	lua->log_user = NULL;
	lua->ui_images = NULL;
	lua->ui_count = 0;
	lua->ui_cap = 0;
	register_engine_api(lua);
	return (1);
}

void	lua_engine_set_prefabs(t_lua_engine *lua, t_prefab_db *prefabs)
{
	if (!lua)
		return ;
	lua->prefabs = prefabs;
}

void	lua_engine_set_logger(t_lua_engine *lua,
		void (*fn)(void *user, const char *line), void *user)
{
	if (!lua)
		return ;
	lua->log_fn = fn;
	lua->log_user = user;
}

void	lua_engine_destroy(t_lua_engine *lua)
{
	if (!lua)
		return ;
	lua_engine_begin_frame(lua);
	free(lua->ui_images);
	lua->ui_images = NULL;
	lua->ui_cap = 0;
	if (lua->L)
		lua_close((lua_State *)lua->L);
	ft_bzero(lua, sizeof(*lua));
}

void	lua_engine_set_mlx(t_lua_engine *lua, mlx_t *mlx)
{
	if (!lua)
		return ;
	lua->mlx = mlx;
}

void	lua_engine_set_world(t_lua_engine *lua, t_file *world)
{
	if (!lua)
		return ;
	lua->world = world;
}

void	lua_engine_set_player_id(t_lua_engine *lua, uint32_t id)
{
	if (!lua)
		return ;
	lua->player_id = id;
}

void	lua_engine_begin_frame(t_lua_engine *lua)
{
	int		i;
	mlx_image_t	*img;

	if (!lua || !lua->mlx || !lua->ui_images)
		return ;
	i = 0;
	while (i < lua->ui_count)
	{
		img = (mlx_image_t *)lua->ui_images[i];
		if (img)
			mlx_delete_image(lua->mlx, img);
		i++;
	}
	lua->ui_count = 0;
}

static int	setup_env(lua_State *L)
{
	/* env */
	lua_newtable(L);
	/* metatable: __index = _G */
	lua_newtable(L);
	lua_pushglobaltable(L);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	/* env.engine = _G.engine */
	lua_getglobal(L, "engine");
	lua_setfield(L, -2, "engine");
	return (1);
}

int	lua_script_load(t_lua_engine *lua, t_lua_script *out, const char *path)
{
	lua_State	*L;
	int			status;
	int			env_ref;

	if (!lua || !out || !path)
		return (0);
	L = (lua_State *)lua->L;
	ft_bzero(out, sizeof(*out));
	out->path = ft_strdup(path);
	if (!out->path)
		return (0);
	status = luaL_loadfile(L, path);
	if (status != LUA_OK)
		return (lua_log_ex(lua, lua_tostring(L, -1)), lua_pop(L, 1),
			free(out->path), 0);
	setup_env(L);
	/* stack: chunk, env */
	lua_pushvalue(L, -1);
	env_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_setupvalue(L, -2, 1);
	status = lua_pcall(L, 0, 0, 0);
	if (status != LUA_OK)
		return (luaL_unref(L, LUA_REGISTRYINDEX, env_ref),
			lua_log_ex(lua, lua_tostring(L, -1)), lua_pop(L, 1),
			free(out->path), 0);
	out->env_ref = env_ref;
	return (1);
}

void	lua_script_unload(t_lua_engine *lua, t_lua_script *script)
{
	lua_State	*L;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	if (script->env_ref != 0)
		luaL_unref(L, LUA_REGISTRYINDEX, script->env_ref);
	free(script->path);
	ft_bzero(script, sizeof(*script));
}

static void	call_entity_fn(t_lua_engine *lua, t_lua_script *script,
					const char *fn, int nargs)
{
	lua_State	*L;
	int			t;

	if (!lua || !script || script->env_ref == 0)
		return ;
	L = (lua_State *)lua->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, script->env_ref);
	lua_getfield(L, -1, fn);
	t = lua_type(L, -1);
	if (t != LUA_TFUNCTION)
		return (lua_pop(L, 2));
	/* stack: args..., env, fn */
	lua_remove(L, -2);
	/* stack: args..., fn */
	lua_insert(L, -1 - nargs);
	if (lua_pcall(L, nargs, 0, 0) != LUA_OK)
	{
		lua_log_ex(lua, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static int	push_exports_table(lua_State *L, int env_idx)
{
	int		pairs;

	if (!L)
		return (0);
	lua_getfield(L, env_idx, "export");
	if (lua_type(L, -1) == LUA_TTABLE)
		return (1);
	lua_pop(L, 1);
	lua_pushnil(L);
	pairs = 0;
	while (lua_next(L, env_idx) != 0)
	{
		pairs++;
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			lua_getfield(L, -1, "export");
			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_remove(L, -2);
				lua_remove(L, -2);
				return (1);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
		if (pairs > 128)
			break ;
	}
	return (0);
}

int	lua_script_apply_exports(t_lua_engine *lua, t_lua_script *script,
			t_entity *ent, int overwrite)
{
	lua_State	*L;
	int			env_idx;
	int			exp_idx;
	const char		*k;
	int			t;
	t_property		*cur;

	if (!lua || !script || !ent || script->env_ref == 0)
		return (0);
	L = (lua_State *)lua->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, script->env_ref);
	env_idx = lua_gettop(L);
	if (!push_exports_table(L, env_idx))
		return (lua_pop(L, 1), 1);
	exp_idx = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, exp_idx) != 0)
	{
		k = lua_tostring(L, -2);
		t = lua_type(L, -1);
		cur = NULL;
		if (k && k[0] == '_' && k[1] == '_')
			cur = (t_property *)1;
		if (k && !cur && !overwrite)
			cur = prop_get(ent->properties, k);
		if (k && !cur)
		{
			if (t == LUA_TNUMBER)
				prop_set_number(&ent->properties, k,
					(double)lua_tonumber(L, -1));
			else if (t == LUA_TBOOLEAN)
				prop_set_bool(&ent->properties, k,
					(int)lua_toboolean(L, -1));
			else if (t == LUA_TSTRING)
				prop_set_string(&ent->properties, k,
					lua_tostring(L, -1));
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	return (1);
}

static void	lua_script_sync_exports_from_props(t_lua_engine *lua,
		t_lua_script *script, t_entity *ent)
{
	lua_State	*L;
	int			env_idx;
	int			exp_idx;
	t_property	*p;

	if (!lua || !script || !ent || script->env_ref == 0 || !lua->L)
		return ;
	L = (lua_State *)lua->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, script->env_ref);
	env_idx = lua_gettop(L);
	if (!push_exports_table(L, env_idx))
		return ((void)lua_pop(L, 1));
	exp_idx = lua_gettop(L);
	p = ent->properties;
	while (p)
	{
		if (!p->key || (p->key[0] == '_' && p->key[1] == '_'))
		{
			p = p->next;
			continue ;
		}
		lua_getfield(L, exp_idx, p->key);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			p = p->next;
			continue ;
		}
		lua_pop(L, 1);
		if (p->type == PROP_NUMBER)
			lua_pushnumber(L, (lua_Number)p->v.n);
		else if (p->type == PROP_BOOL)
			lua_pushboolean(L, p->v.b != 0);
		else if (p->type == PROP_STRING)
			lua_pushstring(L, p->v.s ? p->v.s : "");
		else
		{
			p = p->next;
			continue ;
		}
		lua_setfield(L, exp_idx, p->key);
		p = p->next;
	}
	lua_pop(L, 2);
}

void	lua_call_init(t_lua_engine *lua, t_lua_script *script, uint32_t id)
{
	lua_State	*L;
	t_entity	*ent;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	ent = NULL;
	if (lua->store)
		ent = entity_get(lua->store, id);
	if (ent)
		lua_script_sync_exports_from_props(lua, script, ent);
	lua_pushinteger(L, (lua_Integer)id);
	call_entity_fn(lua, script, "init", 1);
}

void	lua_call_ready(t_lua_engine *lua, t_lua_script *script, uint32_t id)
{
	lua_State	*L;
	t_entity	*ent;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	ent = NULL;
	if (lua->store)
		ent = entity_get(lua->store, id);
	if (ent)
		lua_script_sync_exports_from_props(lua, script, ent);
	lua_pushinteger(L, (lua_Integer)id);
	call_entity_fn(lua, script, "ready", 1);
}

void	lua_call_update(t_lua_engine *lua, t_lua_script *script,
		uint32_t id, double dt)
{
	lua_State	*L;
	t_entity	*ent;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	ent = NULL;
	if (lua->store)
		ent = entity_get(lua->store, id);
	if (ent)
		lua_script_sync_exports_from_props(lua, script, ent);
	lua_pushinteger(L, (lua_Integer)id);
	lua_pushnumber(L, (lua_Number)dt);
	call_entity_fn(lua, script, "update", 2);
}

void	lua_call_destroy(t_lua_engine *lua, t_lua_script *script, uint32_t id)
{
	lua_State	*L;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	lua_pushinteger(L, (lua_Integer)id);
	call_entity_fn(lua, script, "destroy", 1);
}
