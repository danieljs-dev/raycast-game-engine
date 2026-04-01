/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   engine.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/engine.h"

#include <stdlib.h>
#include <math.h>

#include "engine/json.h"
#include "engine/scene/scene_io.h"

static void	engine_zero(t_engine *engine)
{
	ft_bzero(engine, sizeof(*engine));
	engine->player_id = 0;
	engine->project_path = NULL;
	engine->prefabs_dir = NULL;
	engine->player_prefab = NULL;
	engine->scene_path = NULL;
}

int	engine_init(t_engine *engine)
{
	if (!engine)
		return (0);
	engine_zero(engine);
	if (!scene_init(&engine->scene))
		return (0);
	if (!prefab_db_init(&engine->prefabs))
		return (scene_destroy(&engine->scene), 0);
	if (!lua_engine_init(&engine->lua, &engine->scene.store))
		return (prefab_db_destroy(&engine->prefabs), scene_destroy(&engine->scene), 0);
	return (1);
}

void	engine_destroy(t_engine *engine)
{
	if (!engine)
		return ;
	lua_engine_destroy(&engine->lua);
	prefab_db_destroy(&engine->prefabs);
	scene_destroy(&engine->scene);
	free_file(&engine->file);
	free_textures(&engine->app.tex);
	free(engine->project_path);
	free(engine->prefabs_dir);
	free(engine->player_prefab);
	free(engine->scene_path);
	engine_zero(engine);
}

static int	engine_set_str(char **dst, const char *src)
{
	if (!dst)
		return (0);
	free(*dst);
	*dst = NULL;
	if (!src)
		return (1);
	*dst = ft_strdup(src);
	if (!*dst)
		return (0);
	return (1);
}

static int	set_texture_path(char **dst, const char *src)
{
	int	fd;

	if (!dst || *dst)
		return (0);
	if (!src || *src == '\0')
		return (0);
	if (!ft_has_xpm_extension(src))
		return (0);
	*dst = ft_strdup(src);
	if (!*dst)
		return (0);
	fd = open(*dst, O_RDONLY);
	if (fd < 0)
		return (0);
	close(fd);
	return (1);
}

static int	load_textures(t_app *app, const t_json *tex)
{
	if (!app || !tex)
		return (0);
	ft_bzero(&app->tex, sizeof(app->tex));
	if (!set_texture_path(&app->tex.no, json_as_string(json_obj_get(tex, "NO"))))
		return (0);
	if (!set_texture_path(&app->tex.so, json_as_string(json_obj_get(tex, "SO"))))
		return (0);
	if (!set_texture_path(&app->tex.we, json_as_string(json_obj_get(tex, "WE"))))
		return (0);
	if (!set_texture_path(&app->tex.ea, json_as_string(json_obj_get(tex, "EA"))))
		return (0);
	return (1);
}

static int	load_rgb(t_rgb *dst, const t_json *arr)
{
	if (!dst || !arr || json_type(arr) != JSON_ARRAY)
		return (0);
	if (json_arr_len(arr) != 3)
		return (0);
	dst->r = (int)json_as_number(json_arr_get(arr, 0));
	dst->g = (int)json_as_number(json_arr_get(arr, 1));
	dst->b = (int)json_as_number(json_arr_get(arr, 2));
	if (dst->r < 0 || dst->r > 255)
		return (0);
	if (dst->g < 0 || dst->g > 255)
		return (0);
	if (dst->b < 0 || dst->b > 255)
		return (0);
	return (1);
}

static int	load_colors(t_app *app, const t_json *colors)
{
	if (!app || !colors)
		return (0);
	if (!load_rgb(&app->floor, json_obj_get(colors, "floor")))
		return (0);
	if (!load_rgb(&app->ceiling, json_obj_get(colors, "ceiling")))
		return (0);
	return (1);
}

static int	load_map(t_file *file, const t_json *map)
{
	int		h;
	int		i;
	const char	*line;
	int		w;

	if (!file || !map || json_type(map) != JSON_ARRAY)
		return (0);
	h = json_arr_len(map);
	if (h <= 0)
		return (0);
	file->map = ft_calloc((size_t)h + 1, sizeof(*file->map));
	if (!file->map)
		return (0);
	file->map_height = h;
	file->map_width = 0;
	i = 0;
	while (i < h)
	{
		line = json_as_string(json_arr_get(map, i));
		if (!line)
			return (0);
		file->map[i] = ft_strdup(line);
		if (!file->map[i])
			return (0);
		w = (int)ft_strlen(file->map[i]);
		if (w > file->map_width)
			file->map_width = w;
		i++;
	}
	return (1);
}

static double	spawn_rot(char dir)
{
	if (dir == 'N')
		return (-M_PI_2);
	if (dir == 'S')
		return (M_PI_2);
	if (dir == 'W')
		return (M_PI);
	return (0.0);
}

static void	player_from_transform(t_player *p, t_transform t)
{
	double	dx;
	double	dy;

	p->x = t.x;
	p->y = t.y;
	dx = cos(t.rot);
	dy = sin(t.rot);
	p->dir_x = dx;
	p->dir_y = dy;
	p->plane_x = -p->dir_y * CAMERA_PLANE_LEN;
	p->plane_y = p->dir_x * CAMERA_PLANE_LEN;
}

static int	create_player_entity(t_engine *engine, const char *prefab)
{
	t_entity	*e;
	t_lua_script	*script;

	e = prefab_instantiate(&engine->prefabs, &engine->scene.store, prefab,
		engine->app.player.x, engine->app.player.y);
	if (!e)
		return (0);
	e->transform.rot = spawn_rot(engine->app.spawn_dir);
	engine->player_id = e->id;
	if (e->script_path)
	{
		script = ft_calloc(1, sizeof(*script));
		if (!script)
			return (0);
		if (!lua_script_load(&engine->lua, script, e->script_path))
			return (free(script), 0);
		e->script = script;
		lua_call_init(&engine->lua, script, e->id);
		lua_call_ready(&engine->lua, script, e->id);
	}
	return (1);
}

int	engine_load_project(t_engine *engine, const char *project_path)
{
	t_json		*root;
	const char	*prefabs_dir;
	const char	*player_prefab;
	const char	*scene_path;

	if (!engine || !project_path)
		return (0);
	root = json_parse_file(project_path);
	if (!root)
		return (ft_print_error(json_last_error()), 0);
	if (!engine_set_str(&engine->project_path, project_path))
		return (json_free(root), ft_print_error("malloc failed"), 0);
	ft_bzero(&engine->app, sizeof(engine->app));
	ft_bzero(&engine->file, sizeof(engine->file));
	engine->file.path = NULL;
	if (!load_textures(&engine->app, json_obj_get(root, "textures")))
		return (json_free(root), ft_print_error("invalid textures"), 0);
	if (!load_colors(&engine->app, json_obj_get(root, "colors")))
		return (json_free(root), ft_print_error("invalid colors"), 0);
	if (!load_map(&engine->file, json_obj_get(root, "map")))
		return (json_free(root), ft_print_error("invalid map"), 0);
	if (validate_map_closed(&engine->file) != 0)
		return (json_free(root), 0);
	if (validate_player_spawn(&engine->app, &engine->file) != 0)
		return (json_free(root), 0);
	if (player_init(&engine->app) != 0)
		return (json_free(root), 0);
	prefabs_dir = json_as_string(json_obj_get(root, "prefabs_dir"));
	if (!prefabs_dir)
		prefabs_dir = "prefabs";
	if (!engine_set_str(&engine->prefabs_dir, prefabs_dir))
		return (json_free(root), ft_print_error("malloc failed"), 0);
	if (!prefab_db_load_dir(&engine->prefabs, prefabs_dir))
		return (json_free(root), ft_print_error("failed to load prefabs"), 0);
	player_prefab = json_as_string(json_obj_get(root, "player_prefab"));
	if (!player_prefab)
		player_prefab = "player";
	if (!engine_set_str(&engine->player_prefab, player_prefab))
		return (json_free(root), ft_print_error("malloc failed"), 0);
	if (!create_player_entity(engine, player_prefab))
		return (json_free(root), ft_print_error("failed to create player"), 0);
	scene_path = json_as_string(json_obj_get(root, "scene"));
	if (scene_path && *scene_path)
	{
		if (!engine_set_str(&engine->scene_path, scene_path))
			return (json_free(root), ft_print_error("malloc failed"), 0);
		if (!scene_load_instances(&engine->scene.store, &engine->prefabs,
				&engine->lua, scene_path))
			return (json_free(root), ft_print_error("failed to load scene"), 0);
	}
	json_free(root);
	return (1);
}

static t_json	*rgb_to_json(t_rgb c)
{
	t_json	*arr;

	arr = json_new_array();
	if (!arr)
		return (NULL);
	json_arr_push(arr, json_new_number((double)c.r));
	json_arr_push(arr, json_new_number((double)c.g));
	json_arr_push(arr, json_new_number((double)c.b));
	return (arr);
}

static t_json	*map_to_json(t_file *file)
{
	t_json	*arr;
	int		i;

	if (!file || !file->map)
		return (NULL);
	arr = json_new_array();
	if (!arr)
		return (NULL);
	i = 0;
	while (file->map[i])
	{
		json_arr_push(arr, json_new_string(file->map[i]));
		i++;
	}
	return (arr);
}

int	engine_save_project(t_engine *engine)
{
	t_json	*root;
	t_json	*tex;
	t_json	*colors;
	t_json	*map;

	if (!engine || !engine->project_path)
		return (0);
	root = json_new_object();
	tex = json_new_object();
	colors = json_new_object();
	map = map_to_json(&engine->file);
	if (!root || !tex || !colors || !map)
		return (json_free(root), json_free(tex), json_free(colors), json_free(map),
			0);
	json_obj_set(tex, "NO", json_new_string(engine->app.tex.no));
	json_obj_set(tex, "SO", json_new_string(engine->app.tex.so));
	json_obj_set(tex, "WE", json_new_string(engine->app.tex.we));
	json_obj_set(tex, "EA", json_new_string(engine->app.tex.ea));
	json_obj_set(colors, "floor", rgb_to_json(engine->app.floor));
	json_obj_set(colors, "ceiling", rgb_to_json(engine->app.ceiling));
	json_obj_set(root, "textures", tex);
	json_obj_set(root, "colors", colors);
	json_obj_set(root, "map", map);
	if (engine->prefabs_dir)
		json_obj_set(root, "prefabs_dir", json_new_string(engine->prefabs_dir));
	if (engine->player_prefab)
		json_obj_set(root, "player_prefab", json_new_string(engine->player_prefab));
	if (engine->scene_path)
		json_obj_set(root, "scene", json_new_string(engine->scene_path));
	if (!json_write_file(engine->project_path, root, 1))
		return (json_free(root), 0);
	json_free(root);
	return (1);
}

int	engine_save_scene(t_engine *engine)
{
	if (!engine || !engine->scene_path)
		return (1);
	return (scene_save_instances(&engine->scene.store, engine->scene_path,
			engine->player_id));
}

int	engine_reload_scripts(t_engine *engine)
{
	uint32_t	id;
	t_entity	*e;
	t_lua_script	*s;

	if (!engine)
		return (0);
	id = 1;
	while (id < engine->scene.store.next_id)
	{
		e = entity_get(&engine->scene.store, id);
		if (e && e->script_path)
		{
			if (!e->script)
				e->script = ft_calloc(1, sizeof(t_lua_script));
			if (!e->script)
				return (0);
			s = (t_lua_script *)e->script;
			lua_script_unload(&engine->lua, s);
			if (!lua_script_load(&engine->lua, s, e->script_path))
				return (0);
			lua_call_init(&engine->lua, s, e->id);
			lua_call_ready(&engine->lua, s, e->id);
		}
		id++;
	}
	return (1);
}

void	engine_tick(t_engine *engine, double dt)
{
	t_entity	*player;
	t_lua_script	*script;

	if (!engine)
		return ;
	if (dt > PLAYER_MAX_DT)
		dt = PLAYER_MAX_DT;
	player = entity_get(&engine->scene.store, engine->player_id);
	if (player && player->script)
	{
		script = (t_lua_script *)player->script;
		lua_call_update(&engine->lua, script, player->id, dt);
	}
	if (player)
		player_from_transform(&engine->app.player, player->transform);
}
