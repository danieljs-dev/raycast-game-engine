/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scene_io.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/scene/scene_io.h"

#include <stdlib.h>

#include "engine/json.h"
#include "engine/scripting/lua_engine.h"
#include "libft.h"

static int	load_properties(t_property **dst, const t_json *props)
{
	int			len;
	int			i;
	const char	*k;
	t_json		*v;
	t_json_type	t;

	if (!dst || !props || json_type(props) != JSON_OBJECT)
		return (1);
	len = json_obj_len(props);
	i = 0;
	while (i < len)
	{
		k = json_obj_key_at(props, i);
		v = json_obj_val_at(props, i);
		t = json_type(v);
		if (k && t == JSON_NUMBER)
			prop_set_number(dst, k, json_as_number(v));
		else if (k && t == JSON_BOOL)
			prop_set_bool(dst, k, json_as_bool(v));
		else if (k && t == JSON_STRING)
			prop_set_string(dst, k, json_as_string(v));
		i++;
	}
	return (1);
}

static t_json	*props_to_json(t_property *props)
{
	t_json		*obj;
	t_property	*p;

	obj = json_new_object();
	if (!obj)
		return (NULL);
	p = props;
	while (p)
	{
		if (p->type == PROP_NUMBER)
			json_obj_set(obj, p->key, json_new_number(p->v.n));
		else if (p->type == PROP_BOOL)
			json_obj_set(obj, p->key, json_new_bool(p->v.b));
		else if (p->type == PROP_STRING)
			json_obj_set(obj, p->key, json_new_string(p->v.s));
		p = p->next;
	}
	return (obj);
}

static int	entity_from_json(t_entity_store *store, t_prefab_db *prefabs,
			t_lua_engine *lua, const t_json *j)
{
	const char		*prefab;
	t_entity		*e;
	const t_json	*props;

	if (!store || !prefabs || !j || json_type(j) != JSON_OBJECT)
		return (0);
	prefab = json_as_string(json_obj_get(j, "prefab"));
	if (!prefab)
		prefab = json_as_string(json_obj_get(j, "type"));
	if (!prefab)
		return (0);
	e = prefab_instantiate(prefabs, store, prefab,
		json_as_number(json_obj_get(j, "x")),
		json_as_number(json_obj_get(j, "y")));
	if (!e)
		return (0);
	e->transform.rot = json_as_number(json_obj_get(j, "rot"));
	props = json_obj_get(j, "properties");
	if (!load_properties(&e->properties, props))
		return (0);
	if (lua && e->script_path)
	{
		e->script = ft_calloc(1, sizeof(t_lua_script));
		if (!e->script)
			return (0);
		if (!lua_script_load(lua, (t_lua_script *)e->script, e->script_path))
			return (free(e->script), e->script = NULL, 0);
		lua_call_init(lua, (t_lua_script *)e->script, e->id);
		lua_call_ready(lua, (t_lua_script *)e->script, e->id);
	}
	return (1);
}

int	scene_load_instances(t_entity_store *store, t_prefab_db *prefabs,
		t_lua_engine *lua, const char *scene_path)
{
	t_json		*root;
	const t_json	*arr;
	int			i;
	int			len;

	if (!store || !prefabs || !scene_path)
		return (0);
	root = json_parse_file(scene_path);
	if (!root)
		return (0);
	arr = json_obj_get(root, "entities");
	if (!arr || json_type(arr) != JSON_ARRAY)
		return (json_free(root), 1);
	len = json_arr_len(arr);
	i = 0;
	while (i < len)
	{
		if (!entity_from_json(store, prefabs, lua, json_arr_get(arr, i)))
			return (json_free(root), 0);
		i++;
	}
	json_free(root);
	return (1);
}

static int	push_entity_json(t_json *arr, t_entity *e)
{
	t_json	*obj;
	t_json	*props;

	obj = json_new_object();
	if (!obj)
		return (0);
	json_obj_set(obj, "type", json_new_string(e->type));
	json_obj_set(obj, "x", json_new_number(e->transform.x));
	json_obj_set(obj, "y", json_new_number(e->transform.y));
	json_obj_set(obj, "rot", json_new_number(e->transform.rot));
	props = props_to_json(e->properties);
	if (!props)
		return (json_free(obj), 0);
	json_obj_set(obj, "properties", props);
	if (!json_arr_push(arr, obj))
		return (json_free(obj), 0);
	return (1);
}

int	scene_save_instances(t_entity_store *store, const char *scene_path,
		uint32_t skip_id)
{
	t_json		*root;
	t_json		*arr;
	uint32_t	id;
	t_entity	*e;

	if (!store || !scene_path)
		return (0);
	root = json_new_object();
	arr = json_new_array();
	if (!root || !arr)
		return (json_free(root), json_free(arr), 0);
	json_obj_set(root, "entities", arr);
	id = 1;
	while (id < store->next_id)
	{
		e = entity_get(store, id);
		if (e && e->id != skip_id)
		{
			if (!push_entity_json(arr, e))
				return (json_free(root), 0);
		}
		id++;
	}
	if (!json_write_file(scene_path, root, 1))
		return (json_free(root), 0);
	json_free(root);
	return (1);
}
