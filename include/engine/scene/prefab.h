/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prefab.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PREFAB_H
# define PREFAB_H

# include "engine/scene/entity.h"

typedef struct s_prefab
{
	char			*name;
	char			*script;
	t_property		*defaults;
	struct s_prefab	*next;
} 	t_prefab;

typedef struct s_prefab_db
{
	t_prefab	*head;
} 	t_prefab_db;

int		prefab_db_init(t_prefab_db *db);
void	prefab_db_destroy(t_prefab_db *db);

int		prefab_db_load_dir(t_prefab_db *db, const char *dir_path);
t_prefab	*prefab_db_get(t_prefab_db *db, const char *name);

/* creates entity and copies defaults into entity properties */
t_entity	*prefab_instantiate(t_prefab_db *db, t_entity_store *store,
				const char *prefab_name, double x, double y);

#endif
