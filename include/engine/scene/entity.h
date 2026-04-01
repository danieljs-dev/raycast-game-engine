/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   entity.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENTITY_H
# define ENTITY_H

# include <stdint.h>

typedef struct s_property	t_property;

typedef struct s_transform
{
	double	x;
	double	y;
	double	rot;
} 	t_transform;

typedef struct s_entity
{
	uint32_t		id;
	char			*type;
	char			*script_path;
	t_transform	transform;
	uint32_t		parent_id;
	void			*script;
	t_property		*properties;
} 	t_entity;

typedef struct s_entity_store
{
	t_entity		**items;
	uint32_t		capacity;
	uint32_t		next_id;
	uint32_t		*free_ids;
	uint32_t		free_count;
	uint32_t		free_cap;
} 	t_entity_store;

int		entity_store_init(t_entity_store *store);
void	entity_store_destroy(t_entity_store *store);

t_entity	*entity_create(t_entity_store *store, const char *type);
void		entity_destroy(t_entity_store *store, uint32_t id);
t_entity	*entity_get(t_entity_store *store, uint32_t id);

int		entity_set_type(t_entity *ent, const char *type);
void	entity_set_parent(t_entity *ent, uint32_t parent_id);

void	entity_move(t_entity *ent, double dx, double dy);
void	entity_rotate(t_entity *ent, double drot);

/* properties */
typedef enum e_prop_type
{
	PROP_NIL,
	PROP_BOOL,
	PROP_NUMBER,
	PROP_STRING
} 	t_prop_type;

struct s_property
{
	char			*key;
	t_prop_type		type;
	union
	{
		int		b;
		double	n;
		char	*s;
	} 			v;
	t_property		*next;
};

t_property	*prop_set_number(t_property **lst, const char *key, double v);
t_property	*prop_set_bool(t_property **lst, const char *key, int v);
t_property	*prop_set_string(t_property **lst, const char *key, const char *v);
t_property	*prop_get(t_property *lst, const char *key);
void		prop_free_all(t_property *lst);

#endif
