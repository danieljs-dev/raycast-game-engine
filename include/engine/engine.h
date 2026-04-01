/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   engine.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENGINE_H
# define ENGINE_H

# include "cub3d.h"

# include "engine/scene/scene.h"
# include "engine/scene/prefab.h"
# include "engine/scripting/lua_engine.h"

typedef struct s_engine
{
	t_app		app;
	t_file		file;
	t_scene		scene;
	t_prefab_db	prefabs;
	t_lua_engine	lua;
	uint32_t	player_id;
	char		*project_path;
	char		*prefabs_dir;
	char		*player_prefab;
	char		*scene_path;
} 	t_engine;

int	engine_init(t_engine *engine);
void	engine_destroy(t_engine *engine);

int	engine_load_project(t_engine *engine, const char *project_path);
void	engine_tick(t_engine *engine, double dt);

int	engine_save_project(t_engine *engine);
int	engine_save_scene(t_engine *engine);
int	engine_reload_scripts(t_engine *engine);

#endif
