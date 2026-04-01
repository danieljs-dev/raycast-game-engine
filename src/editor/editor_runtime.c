/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   editor_runtime.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "editor/editor_runtime.h"

#include <stdlib.h>
#include <string.h>

#include "engine/engine.h"

#include "cub3d.h"

static void	rt_log_push(t_editor_runtime *rt, const char *line)
{
	int	idx;

	if (!rt || !line)
		return ;
	idx = rt->log_head % 256;
	strncpy(rt->logs[idx], line, 511);
	rt->logs[idx][511] = '\0';
	rt->log_head = (rt->log_head + 1) % 256;
	if (rt->log_head == rt->log_tail)
		rt->log_tail = (rt->log_tail + 1) % 256;
}

static void	lua_logger_cb(void *user, const char *line)
{
	rt_log_push((t_editor_runtime *)user, line);
}

int	editor_runtime_log_pop(t_editor_runtime *rt, char *out, int out_cap)
{
	int	idx;

	if (!rt || !out || out_cap <= 0)
		return (0);
	if (rt->log_tail == rt->log_head)
		return (0);
	idx = rt->log_tail % 256;
	strncpy(out, rt->logs[idx], (size_t)out_cap - 1);
	out[out_cap - 1] = '\0';
	rt->log_tail = (rt->log_tail + 1) % 256;
	return (1);
}

int	editor_runtime_init(t_editor_runtime *rt, mlx_t *mlx,
		const char *project_path)
{
	t_engine	*engine;

	if (!rt || !mlx || !project_path)
		return (0);
	rt->engine = NULL;
	rt->play_mode = 0;
	rt->log_head = 0;
	rt->log_tail = 0;
	engine = (t_engine *)calloc(1, sizeof(*engine));
	if (!engine)
		return (0);
	if (!engine_init(engine))
		return (free(engine), 0);
	lua_engine_set_logger(&engine->lua, lua_logger_cb, rt);
	if (!engine_load_project(engine, project_path))
		return (engine_destroy(engine), free(engine), 0);
	engine->app.file = &engine->file;
	engine->app.mlx.ptr = mlx;
	engine->app.mlx.win = NULL;
	engine->app.running = 1;
	lua_engine_set_mlx(&engine->lua, mlx);
	if (!framebuffer_init(&engine->app))
		return (engine_destroy(engine), free(engine), 0);
	if (init_loaded_textures(&engine->app) != 0)
		return (framebuffer_destroy(&engine->app), engine_destroy(engine),
			free(engine), 0);
	rt->engine = engine;
	return (1);
}

void	editor_runtime_destroy(t_editor_runtime *rt)
{
	t_engine	*engine;

	if (!rt)
		return ;
	engine = rt->engine;
	if (engine)
	{
		framebuffer_destroy(&engine->app);
		free_loaded_textures(&engine->app);
		if (engine->app.mlx.ptr && engine->app.fps_img)
			mlx_delete_image(engine->app.mlx.ptr, engine->app.fps_img);
		if (engine->app.mlx.ptr && engine->app.ms_img)
			mlx_delete_image(engine->app.mlx.ptr, engine->app.ms_img);
		engine->app.fps_img = NULL;
		engine->app.ms_img = NULL;
		engine_destroy(engine);
		free(engine);
	}
	rt->engine = NULL;
	rt->play_mode = 0;
}

void	editor_runtime_set_play_mode(t_editor_runtime *rt, int enabled)
{
	if (!rt)
		return ;
	rt->play_mode = (enabled != 0);
}

void	editor_runtime_tick(t_editor_runtime *rt, double dt)
{
	if (!rt || !rt->engine)
		return ;
	lua_engine_begin_frame(&rt->engine->lua);
	if (!rt->play_mode)
		return ;
	engine_tick(rt->engine, dt);
}

void	editor_runtime_render(t_editor_runtime *rt)
{
	if (!rt || !rt->engine)
		return ;
	render_frame_core(&rt->engine->app);
}

mlx_image_t	*editor_runtime_frame_image(t_editor_runtime *rt)
{
	if (!rt || !rt->engine)
		return (NULL);
	return ((mlx_image_t *)rt->engine->app.frame.ptr);
}

uint32_t	editor_runtime_player_id(t_editor_runtime *rt)
{
	if (!rt || !rt->engine)
		return (0);
	return (rt->engine->player_id);
}

t_engine	*editor_runtime_engine(t_editor_runtime *rt)
{
	if (!rt)
		return (NULL);
	return (rt->engine);
}
