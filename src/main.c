/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vinda-si <vinda-si@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 21:02:28 by dajesus-          #+#    #+#             */
/*   Updated: 2026/03/11 23:24:35 by vinda-si         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"
#include "engine/engine.h"

static void	app_destroy(t_app *app)
{
	if (!app)
		return ;
	framebuffer_destroy(app);
	free_loaded_textures(app);
	if (app->mlx.ptr && app->fps_img)
		mlx_delete_image(app->mlx.ptr, app->fps_img);
	if (app->mlx.ptr && app->ms_img)
		mlx_delete_image(app->mlx.ptr, app->ms_img);
	app->fps_img = NULL;
	app->ms_img = NULL;
	if (app->mlx.ptr)
		mlx_terminate(app->mlx.ptr);
	app->mlx.ptr = NULL;
}

static int	app_init(t_app *app)
{
	int32_t	inst;

	app->running = 1;
	app->mlx.ptr = mlx_init(CUB3D_WIN_W, CUB3D_WIN_H, CUB3D_WIN_TITLE, true);
	if (!app->mlx.ptr)
		return (0);
	app->mlx.win = NULL;
	if (!framebuffer_init(app))
		return (0);
	inst = mlx_image_to_window(app->mlx.ptr, app->frame.ptr, 0, 0);
	if (inst < 0)
		return (0);
	if (init_loaded_textures(app) != 0)
		return (0);
	return (1);
}

static void	loop_hook(void *param)
{
	t_engine	*engine;

	engine = (t_engine *)param;
	lua_engine_begin_frame(&engine->lua);
	engine_tick(engine, ((mlx_t *)engine->app.mlx.ptr)->delta_time);
	render_frame_core(&engine->app);
}

int	main(int argc, char **argv)
{
	t_engine	engine;
	const char	*project;

	project = "maps/level01.json";
	if (argc == 2)
		project = argv[1];
	if (argc != 1 && argc != 2)
		return (ft_print_error("usage: ./cub3D [project.json]"));
	if (!engine_init(&engine))
		return (ft_print_error("engine init failed"));
	if (!engine_load_project(&engine, project))
		return (engine_destroy(&engine), 1);
	engine.app.file = &engine.file;
	if (!app_init(&engine.app))
	{
		engine_destroy(&engine);
		app_destroy(&engine.app);
		return (1);
	}
	lua_engine_set_mlx(&engine.lua, (mlx_t *)engine.app.mlx.ptr);
	mlx_key_hook(engine.app.mlx.ptr, on_key, &engine.app);
	mlx_close_hook(engine.app.mlx.ptr, on_destroy, &engine.app);
	mlx_loop_hook(engine.app.mlx.ptr, loop_hook, &engine);
	mlx_loop(engine.app.mlx.ptr);
	engine_destroy(&engine);
	app_destroy(&engine.app);
	return (0);
}
