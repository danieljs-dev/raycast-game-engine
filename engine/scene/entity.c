/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   entity.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/scene/entity.h"

#include <stdlib.h>

#include "libft.h"

static void	store_bzero(t_entity_store *store)
{
	ft_bzero(store, sizeof(*store));
	store->next_id = 1;
}

int	entity_store_init(t_entity_store *store)
{
	if (!store)
		return (0);
	store_bzero(store);
	store->capacity = 256;
	store->items = ft_calloc(store->capacity, sizeof(*store->items));
	if (!store->items)
		return (0);
	store->free_cap = 64;
	store->free_ids = ft_calloc(store->free_cap, sizeof(*store->free_ids));
	if (!store->free_ids)
		return (free(store->items), store_bzero(store), 0);
	return (1);
}

static void	entity_free(t_entity *ent)
{
	if (!ent)
		return ;
	free(ent->type);
	free(ent->script_path);
	prop_free_all(ent->properties);
	free(ent);
}

void	entity_store_destroy(t_entity_store *store)
{
	uint32_t	i;

	if (!store)
		return ;
	i = 0;
	while (i < store->capacity)
		entity_free(store->items[i++]);
	free(store->items);
	free(store->free_ids);
	store_bzero(store);
}

static int	store_grow_items(t_entity_store *store, uint32_t need_id)
{
	t_entity	**new_items;
	uint32_t	new_cap;

	new_cap = store->capacity;
	while (need_id >= new_cap)
		new_cap *= 2;
	new_items = ft_calloc(new_cap, sizeof(*new_items));
	if (!new_items)
		return (0);
	ft_memcpy(new_items, store->items, sizeof(*new_items) * store->capacity);
	free(store->items);
	store->items = new_items;
	store->capacity = new_cap;
	return (1);
}

static int	store_grow_free_ids(t_entity_store *store)
{
	uint32_t	*new_ids;
	uint32_t	new_cap;

	new_cap = store->free_cap * 2;
	if (new_cap < 64)
		new_cap = 64;
	new_ids = ft_calloc(new_cap, sizeof(*new_ids));
	if (!new_ids)
		return (0);
	ft_memcpy(new_ids, store->free_ids,
		sizeof(*new_ids) * store->free_count);
	free(store->free_ids);
	store->free_ids = new_ids;
	store->free_cap = new_cap;
	return (1);
}

static uint32_t	store_alloc_id(t_entity_store *store)
{
	uint32_t	id;

	if (store->free_count > 0)
	{
		id = store->free_ids[store->free_count - 1];
		store->free_count--;
		return (id);
	}
	id = store->next_id;
	store->next_id++;
	return (id);
}

t_entity	*entity_create(t_entity_store *store, const char *type)
{
	t_entity	*ent;
	uint32_t	id;

	if (!store)
		return (NULL);
	id = store_alloc_id(store);
	if (id >= store->capacity)
		if (!store_grow_items(store, id))
			return (NULL);
	ent = ft_calloc(1, sizeof(*ent));
	if (!ent)
		return (NULL);
	ent->id = id;
	ent->parent_id = 0;
	ent->script_path = NULL;
	ent->transform.x = 0.0;
	ent->transform.y = 0.0;
	ent->transform.rot = 0.0;
	if (type)
	{
		ent->type = ft_strdup(type);
		if (!ent->type)
			return (free(ent), NULL);
	}
	store->items[id] = ent;
	return (ent);
}

void	entity_destroy(t_entity_store *store, uint32_t id)
{
	if (!store || id == 0 || id >= store->capacity)
		return ;
	if (!store->items[id])
		return ;
	entity_free(store->items[id]);
	store->items[id] = NULL;
	if (store->free_count + 1 > store->free_cap)
		if (!store_grow_free_ids(store))
			return ;
	store->free_ids[store->free_count++] = id;
}

t_entity	*entity_get(t_entity_store *store, uint32_t id)
{
	if (!store || id == 0 || id >= store->capacity)
		return (NULL);
	return (store->items[id]);
}

int	entity_set_type(t_entity *ent, const char *type)
{
	char	*dup;

	if (!ent)
		return (0);
	dup = NULL;
	if (type)
	{
		dup = ft_strdup(type);
		if (!dup)
			return (0);
	}
	free(ent->type);
	ent->type = dup;
	return (1);
}

void	entity_set_parent(t_entity *ent, uint32_t parent_id)
{
	if (!ent)
		return ;
	ent->parent_id = parent_id;
}

void	entity_move(t_entity *ent, double dx, double dy)
{
	if (!ent)
		return ;
	ent->transform.x += dx;
	ent->transform.y += dy;
}

void	entity_rotate(t_entity *ent, double drot)
{
	if (!ent)
		return ;
	ent->transform.rot += drot;
}

static t_property	*prop_find(t_property *lst, const char *key)
{
	while (lst)
	{
		if (lst->key && key && ft_strcmp(lst->key, key) == 0)
			return (lst);
		lst = lst->next;
	}
	return (NULL);
}

t_property	*prop_get(t_property *lst, const char *key)
{
	return (prop_find(lst, key));
}

static t_property	*prop_new(const char *key)
{
	t_property	*p;

	p = ft_calloc(1, sizeof(*p));
	if (!p)
		return (NULL);
	p->key = ft_strdup(key);
	if (!p->key)
		return (free(p), NULL);
	p->type = PROP_NIL;
	p->next = NULL;
	return (p);
}

static t_property	*prop_ensure(t_property **lst, const char *key)
{
	t_property	*p;

	if (!lst || !key)
		return (NULL);
	p = prop_find(*lst, key);
	if (p)
		return (p);
	p = prop_new(key);
	if (!p)
		return (NULL);
	p->next = *lst;
	*lst = p;
	return (p);
}

static void	prop_clear_value(t_property *p)
{
	if (!p)
		return ;
	if (p->type == PROP_STRING)
		free(p->v.s);
	ft_bzero(&p->v, sizeof(p->v));
	p->type = PROP_NIL;
}

t_property	*prop_set_number(t_property **lst, const char *key, double v)
{
	t_property	*p;

	p = prop_ensure(lst, key);
	if (!p)
		return (NULL);
	prop_clear_value(p);
	p->type = PROP_NUMBER;
	p->v.n = v;
	return (p);
}

t_property	*prop_set_bool(t_property **lst, const char *key, int v)
{
	t_property	*p;

	p = prop_ensure(lst, key);
	if (!p)
		return (NULL);
	prop_clear_value(p);
	p->type = PROP_BOOL;
	p->v.b = (v != 0);
	return (p);
}

t_property	*prop_set_string(t_property **lst, const char *key, const char *v)
{
	t_property	*p;
	char		*dup;

	p = prop_ensure(lst, key);
	if (!p)
		return (NULL);
	dup = NULL;
	if (v)
	{
		dup = ft_strdup(v);
		if (!dup)
			return (NULL);
	}
	prop_clear_value(p);
	p->type = PROP_STRING;
	p->v.s = dup;
	return (p);
}

void	prop_free_all(t_property *lst)
{
	t_property	*n;

	while (lst)
	{
		n = lst->next;
		free(lst->key);
		if (lst->type == PROP_STRING)
			free(lst->v.s);
		free(lst);
		lst = n;
	}
}
