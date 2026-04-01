/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   json.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef JSON_H
# define JSON_H

typedef enum e_json_type
{
	JSON_NULL,
	JSON_BOOL,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
} 	t_json_type;

typedef struct s_json	t_json;

t_json	*json_parse_file(const char *path);
t_json	*json_parse_str(const char *src);
void	json_free(t_json *json);

/* build + write */
t_json	*json_new_null(void);
t_json	*json_new_bool(int v);
t_json	*json_new_number(double v);
t_json	*json_new_string(const char *s);
t_json	*json_new_array(void);
t_json	*json_new_object(void);

int		json_arr_push(t_json *arr, t_json *value);
int		json_obj_set(t_json *obj, const char *key, t_json *value);

char	*json_stringify(const t_json *json, int pretty);
int		json_write_file(const char *path, const t_json *json, int pretty);

t_json_type	json_type(const t_json *json);
const char	*json_as_string(const t_json *json);
double		json_as_number(const t_json *json);
int			json_as_bool(const t_json *json);

t_json	*json_obj_get(const t_json *obj, const char *key);
t_json	*json_obj_val_at(const t_json *obj, int index);
const char	*json_obj_key_at(const t_json *obj, int index);
int		json_obj_len(const t_json *obj);
t_json	*json_arr_get(const t_json *arr, int index);
int		json_arr_len(const t_json *arr);

const char	*json_last_error(void);

#endif
