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

#include "engine/engine.h"

#include "cub3d.h"

int	editor_runtime_init(t_editor_runtime *rt, mlx_t *mlx,
		const char *project_path)
{
	t_engine	*engine;

	if (!rt || !mlx || !project_path)
		return (0);
	rt->engine = NULL;
	rt->play_mode = 0;
	engine = (t_engine *)calloc(1, sizeof(*engine));
	if (!engine)
		return (0);
	if (!engine_init(engine))
		return (free(engine), 0);
	if (!engine_load_project(engine, project_path))
		return (engine_destroy(engine), free(engine), 0);
	engine->app.file = &engine->file;
	engine->app.mlx.ptr = mlx;
	engine->app.mlx.win = NULL;
	engine->app.running = 1;
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
