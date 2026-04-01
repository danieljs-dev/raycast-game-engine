/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/scene/scene.h"

int	scene_init(t_scene *scene)
{
	t_entity	*root;

	if (!scene)
		return (0);
	if (!entity_store_init(&scene->store))
		return (0);
	root = entity_create(&scene->store, "root");
	if (!root)
		return (entity_store_destroy(&scene->store), 0);
	scene->root_id = root->id;
	return (1);
}

void	scene_destroy(t_scene *scene)
{
	if (!scene)
		return ;
	entity_store_destroy(&scene->store);
	scene->root_id = 0;
}

t_entity	*scene_root(t_scene *scene)
{
	if (!scene)
		return (0);
	return (entity_get(&scene->store, scene->root_id));
}
