/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lua_engine.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LUA_ENGINE_H
# define LUA_ENGINE_H

# include <stdint.h>

typedef struct s_lua_engine	t_lua_engine;
typedef struct s_entity_store	t_entity_store;

typedef struct s_lua_script
{
	char	*path;
	int		env_ref;
} 	t_lua_script;

struct s_lua_engine
{
	void			*L;
	t_entity_store	*store;
};

int	lua_engine_init(t_lua_engine *lua, t_entity_store *store);
void	lua_engine_destroy(t_lua_engine *lua);

int	lua_script_load(t_lua_engine *lua, t_lua_script *out, const char *path);
void	lua_script_unload(t_lua_engine *lua, t_lua_script *script);

void	lua_call_init(t_lua_engine *lua, t_lua_script *script, uint32_t id);
void	lua_call_ready(t_lua_engine *lua, t_lua_script *script, uint32_t id);
void	lua_call_update(t_lua_engine *lua, t_lua_script *script,
		uint32_t id, double dt);
void	lua_call_destroy(t_lua_engine *lua, t_lua_script *script, uint32_t id);

#endif
