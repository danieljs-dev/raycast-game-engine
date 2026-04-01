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

#include "engine/scene/entity.h"
#include "libft.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static void	lua_log(const char *msg)
{
	if (!msg)
		return ;
	write(2, msg, ft_strlen(msg));
	write(2, "\n", 1);
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
		entity_move(e, dx, dy);
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
	lua_setfield(L, -2, "entity");
	lua_setglobal(L, "engine");
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
	register_engine_api(lua);
	return (1);
}

void	lua_engine_destroy(t_lua_engine *lua)
{
	if (!lua)
		return ;
	if (lua->L)
		lua_close((lua_State *)lua->L);
	ft_bzero(lua, sizeof(*lua));
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
		return (lua_log(lua_tostring(L, -1)), lua_pop(L, 1), free(out->path), 0);
	setup_env(L);
	/* stack: chunk, env */
	lua_pushvalue(L, -1);
	env_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_setupvalue(L, -2, 1);
	status = lua_pcall(L, 0, 0, 0);
	if (status != LUA_OK)
		return (luaL_unref(L, LUA_REGISTRYINDEX, env_ref),
			lua_log(lua_tostring(L, -1)), lua_pop(L, 1), free(out->path), 0);
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
		lua_log(lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void	lua_call_init(t_lua_engine *lua, t_lua_script *script, uint32_t id)
{
	lua_State	*L;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	lua_pushinteger(L, (lua_Integer)id);
	call_entity_fn(lua, script, "init", 1);
}

void	lua_call_ready(t_lua_engine *lua, t_lua_script *script, uint32_t id)
{
	lua_State	*L;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
	lua_pushinteger(L, (lua_Integer)id);
	call_entity_fn(lua, script, "ready", 1);
}

void	lua_call_update(t_lua_engine *lua, t_lua_script *script,
		uint32_t id, double dt)
{
	lua_State	*L;

	if (!lua || !script)
		return ;
	L = (lua_State *)lua->L;
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
