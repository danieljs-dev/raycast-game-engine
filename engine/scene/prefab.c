/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prefab.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/scene/prefab.h"

#include <stdlib.h>
#include <dirent.h>

#include "engine/json.h"
#include "libft.h"

static void	prefab_free(t_prefab *p)
{
	if (!p)
		return ;
	free(p->name);
	free(p->script);
	prop_free_all(p->defaults);
	free(p);
}

int	prefab_db_init(t_prefab_db *db)
{
	if (!db)
		return (0);
	db->head = NULL;
	return (1);
}

void	prefab_db_destroy(t_prefab_db *db)
{
	t_prefab	*n;

	if (!db)
		return ;
	while (db->head)
	{
		n = db->head->next;
		prefab_free(db->head);
		db->head = n;
	}
}

static int	has_json_ext(const char *name)
{
	size_t	len;

	if (!name)
		return (0);
	len = ft_strlen(name);
	if (len < 5)
		return (0);
	return (ft_strncmp(name + (len - 5), ".json", 5) == 0);
}

static char	*join_path(const char *dir, const char *file)
{
	char	*tmp;
	char	*out;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	out = ft_strjoin(tmp, file);
	free(tmp);
	return (out);
}

static t_property	*props_from_json(const t_json *obj)
{
	t_property	*lst;
	int			i;
	int			len;
	const char	*k;
	t_json		*v;
	lst = NULL;
	if (!obj || json_type(obj) != JSON_OBJECT)
		return (NULL);
	len = json_obj_len(obj);
	i = 0;
	while (i < len)
	{
		k = json_obj_key_at(obj, i);
		v = json_obj_val_at(obj, i);
		if (!k || !v)
		{
			i++;
			continue ;
		}
		if (json_type(v) == JSON_NUMBER)
			prop_set_number(&lst, k, json_as_number(v));
		else if (json_type(v) == JSON_BOOL)
			prop_set_bool(&lst, k, json_as_bool(v));
		else if (json_type(v) == JSON_STRING)
			prop_set_string(&lst, k, json_as_string(v));
		i++;
	}
	return (lst);
}

static int	prefab_load_file(t_prefab_db *db, const char *path)
{
	t_json		*root;
	t_prefab	*p;
	const char	*name;
	const char	*script;
	const char	*type;

	root = json_parse_file(path);
	if (!root)
		return (0);
	name = json_as_string(json_obj_get(root, "name"));
	script = json_as_string(json_obj_get(root, "script"));
	type = json_as_string(json_obj_get(root, "type"));
	if (!name || !type)
		return (json_free(root), 0);
	p = ft_calloc(1, sizeof(*p));
	if (!p)
		return (json_free(root), 0);
	p->name = ft_strdup(name);
	p->script = NULL;
	if (script)
		p->script = ft_strdup(script);
	if (!p->name || (script && !p->script))
		return (json_free(root), prefab_free(p), 0);
	/* defaults: {"list":[{"k":"hp","v":100}] } */
	p->defaults = props_from_json(json_obj_get(root, "properties"));
	/* store entity type as a property to keep prefab struct minimal */
	prop_set_string(&p->defaults, "__type", type);
	p->next = db->head;
	db->head = p;
	json_free(root);
	return (1);
}

int	prefab_db_load_dir(t_prefab_db *db, const char *dir_path)
{
	DIR			*dir;
	struct dirent	*ent;
	char			*path;
	int				ok;

	if (!db || !dir_path)
		return (0);
	dir = opendir(dir_path);
	if (!dir)
		return (0);
	ok = 1;
	ent = readdir(dir);
	while (ent)
	{
		if (has_json_ext(ent->d_name))
		{
			path = join_path(dir_path, ent->d_name);
			if (!path)
				ok = 0;
			else if (!prefab_load_file(db, path))
				ok = 0;
			free(path);
		}
		ent = readdir(dir);
	}
	closedir(dir);
	return (ok);
}

t_prefab	*prefab_db_get(t_prefab_db *db, const char *name)
{
	t_prefab	*p;

	if (!db || !name)
		return (NULL);
	p = db->head;
	while (p)
	{
		if (p->name && ft_strcmp(p->name, name) == 0)
			return (p);
		p = p->next;
	}
	return (NULL);
}

static void	props_copy(t_property **dst, t_property *src)
{
	while (src)
	{
		if (src->type == PROP_NUMBER)
			prop_set_number(dst, src->key, src->v.n);
		else if (src->type == PROP_BOOL)
			prop_set_bool(dst, src->key, src->v.b);
		else if (src->type == PROP_STRING)
			prop_set_string(dst, src->key, src->v.s);
		src = src->next;
	}
}

t_entity	*prefab_instantiate(t_prefab_db *db, t_entity_store *store,
				const char *prefab_name, double x, double y)
{
	t_prefab	*p;
	t_entity	*e;
	t_property	*tprop;

	if (!db || !store || !prefab_name)
		return (NULL);
	p = prefab_db_get(db, prefab_name);
	if (!p)
		return (NULL);
	tprop = prop_get(p->defaults, "__type");
	e = entity_create(store, NULL);
	if (!e)
		return (NULL);
	if (tprop && tprop->type == PROP_STRING)
		if (!entity_set_type(e, tprop->v.s))
			return (entity_destroy(store, e->id), NULL);
	e->transform.x = x;
	e->transform.y = y;
	props_copy(&e->properties, p->defaults);
	if (p->script)
	{
		e->script_path = ft_strdup(p->script);
		if (!e->script_path)
			return (entity_destroy(store, e->id), NULL);
	}
	return (e);
}
