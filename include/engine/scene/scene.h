/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENE_H
# define SCENE_H

# include <stdint.h>

# include "engine/scene/entity.h"

typedef struct s_scene
{
	t_entity_store	store;
	uint32_t		root_id;
} 	t_scene;

int		scene_init(t_scene *scene);
void	scene_destroy(t_scene *scene);

t_entity	*scene_root(t_scene *scene);

#endif
