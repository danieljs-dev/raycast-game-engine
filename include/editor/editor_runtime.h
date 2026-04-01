/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   editor_runtime.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EDITOR_RUNTIME_H
# define EDITOR_RUNTIME_H

# include <stdint.h>

typedef struct mlx	mlx_t;
typedef struct mlx_image	mlx_image_t;

typedef struct s_engine	t_engine;

typedef struct s_editor_runtime
{
	t_engine	*engine;
	int			play_mode;
	char		logs[256][512];
	int			log_head;
	int			log_tail;
} 	t_editor_runtime;

int		editor_runtime_init(t_editor_runtime *rt, mlx_t *mlx,
			const char *project_path);
void	editor_runtime_destroy(t_editor_runtime *rt);

void	editor_runtime_set_play_mode(t_editor_runtime *rt, int enabled);

void	editor_runtime_tick(t_editor_runtime *rt, double dt);
void	editor_runtime_render(t_editor_runtime *rt);

mlx_image_t	*editor_runtime_frame_image(t_editor_runtime *rt);
uint32_t		editor_runtime_player_id(t_editor_runtime *rt);
t_engine		*editor_runtime_engine(t_editor_runtime *rt);

int			editor_runtime_log_pop(t_editor_runtime *rt, char *out, int out_cap);

#endif
