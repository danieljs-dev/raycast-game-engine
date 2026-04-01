/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_io.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENE_IO_H
# define SCENE_IO_H

# include "engine/scene/entity.h"
# include "engine/scene/prefab.h"

typedef struct s_lua_engine	t_lua_engine;

int	scene_load_instances(t_entity_store *store, t_prefab_db *prefabs,
		t_lua_engine *lua, const char *scene_path);

int	scene_save_instances(t_entity_store *store, const char *scene_path,
		uint32_t skip_id);

#endif
