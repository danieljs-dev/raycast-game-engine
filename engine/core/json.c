/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   json.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "engine/json.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "libft.h"

typedef struct s_json_member
{
	char				*key;
	struct s_json		*value;
	struct s_json_member	*next;
} 	t_json_member;

struct s_json
{
	t_json_type	type;
	union
	{
		int		b;
		double	n;
		char	*s;
		struct
		{
			struct s_json	**items;
			int			len;
			int			cap;
		} 			arr;
		t_json_member	*obj;
	} 			u;
};

static char	*g_json_err;

static void	set_err(const char *msg)
{
	g_json_err = (char *)msg;
}

const char	*json_last_error(void)
{
	if (!g_json_err)
		return ("unknown json error");
	return (g_json_err);
}

static const char	*skip_ws(const char *s)
{
	while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
		s++;
	return (s);
}

static t_json	*json_new(t_json_type t)
{
	t_json	*j;

	j = ft_calloc(1, sizeof(*j));
	if (!j)
		return (set_err("malloc failed"), NULL);
	j->type = t;
	return (j);
}

static void	member_free_all(t_json_member *m)
{
	t_json_member	*n;

	while (m)
	{
		n = m->next;
		free(m->key);
		json_free(m->value);
		free(m);
		m = n;
	}
}

void	json_free(t_json *json)
{
	int				i;

	if (!json)
		return ;
	if (json->type == JSON_STRING)
		free(json->u.s);
	else if (json->type == JSON_ARRAY)
	{
		i = 0;
		while (i < json->u.arr.len)
			json_free(json->u.arr.items[i++]);
		free(json->u.arr.items);
	}
	else if (json->type == JSON_OBJECT)
		member_free_all(json->u.obj);
	free(json);
}

t_json_type	json_type(const t_json *json)
{
	if (!json)
		return (JSON_NULL);
	return (json->type);
}

const char	*json_as_string(const t_json *json)
{
	if (!json || json->type != JSON_STRING)
		return (NULL);
	return (json->u.s);
}

double	json_as_number(const t_json *json)
{
	if (!json || json->type != JSON_NUMBER)
		return (0.0);
	return (json->u.n);
}

int	json_as_bool(const t_json *json)
{
	if (!json || json->type != JSON_BOOL)
		return (0);
	return (json->u.b);
}

static int	arr_push(t_json *arr, t_json *item)
{
	t_json	**new_items;
	int		new_cap;

	if (!arr || arr->type != JSON_ARRAY)
		return (0);
	if (arr->u.arr.len + 1 > arr->u.arr.cap)
	{
		new_cap = arr->u.arr.cap * 2;
		if (new_cap < 8)
			new_cap = 8;
		new_items = ft_calloc(new_cap, sizeof(*new_items));
		if (!new_items)
			return (set_err("malloc failed"), 0);
		if (arr->u.arr.items)
			ft_memcpy(new_items, arr->u.arr.items,
				sizeof(*new_items) * arr->u.arr.len);
		free(arr->u.arr.items);
		arr->u.arr.items = new_items;
		arr->u.arr.cap = new_cap;
	}
	arr->u.arr.items[arr->u.arr.len++] = item;
	return (1);
}

int	json_arr_push(t_json *arr, t_json *value)
{
	if (!arr || arr->type != JSON_ARRAY || !value)
		return (0);
	return (arr_push(arr, value));
}

int	json_arr_len(const t_json *arr)
{
	if (!arr || arr->type != JSON_ARRAY)
		return (0);
	return (arr->u.arr.len);
}

t_json	*json_arr_get(const t_json *arr, int index)
{
	if (!arr || arr->type != JSON_ARRAY)
		return (NULL);
	if (index < 0 || index >= arr->u.arr.len)
		return (NULL);
	return (arr->u.arr.items[index]);
}

t_json	*json_obj_get(const t_json *obj, const char *key)
{
	t_json_member	*m;

	if (!obj || obj->type != JSON_OBJECT || !key)
		return (NULL);
	m = obj->u.obj;
	while (m)
	{
		if (m->key && ft_strcmp(m->key, key) == 0)
			return (m->value);
		m = m->next;
	}
	return (NULL);
}

int	json_obj_len(const t_json *obj)
{
	t_json_member	*m;
	int				len;

	if (!obj || obj->type != JSON_OBJECT)
		return (0);
	len = 0;
	m = obj->u.obj;
	while (m)
	{
		len++;
		m = m->next;
	}
	return (len);
}

static t_json_member	*obj_member_at(const t_json *obj, int index)
{
	t_json_member	*m;
	int				i;

	if (!obj || obj->type != JSON_OBJECT)
		return (NULL);
	if (index < 0)
		return (NULL);
	m = obj->u.obj;
	i = 0;
	while (m)
	{
		if (i == index)
			return (m);
		i++;
		m = m->next;
	}
	return (NULL);
}

const char	*json_obj_key_at(const t_json *obj, int index)
{
	t_json_member	*m;

	m = obj_member_at(obj, index);
	if (!m)
		return (NULL);
	return (m->key);
}

t_json	*json_obj_val_at(const t_json *obj, int index)
{
	t_json_member	*m;

	m = obj_member_at(obj, index);
	if (!m)
		return (NULL);
	return (m->value);
}

static const char	*parse_str(const char *s, char **out)
{
	char	*buf;
	int		len;
	int		cap;
	char	c;

	if (*s != '"')
		return (set_err("expected string"), NULL);
	s++;
	cap = 32;
	len = 0;
	buf = ft_calloc(cap, sizeof(*buf));
	if (!buf)
		return (set_err("malloc failed"), NULL);
	while (*s && *s != '"')
	{
		c = *s++;
		if (c == '\\')
		{
			c = *s++;
			if (c == 'n')
				c = '\n';
			else if (c == 't')
				c = '\t';
			else if (c == 'r')
				c = '\r';
			else if (c == '"')
				c = '"';
			else if (c == '\\')
				c = '\\';
			else
				return (free(buf), set_err("unsupported escape"), NULL);
		}
		if (len + 2 > cap)
		{
			char	*new_buf;
			cap *= 2;
			new_buf = ft_calloc(cap, sizeof(*new_buf));
			if (!new_buf)
				return (free(buf), set_err("malloc failed"), NULL);
			ft_memcpy(new_buf, buf, len);
			free(buf);
			buf = new_buf;
		}
		buf[len++] = c;
	}
	if (*s != '"')
		return (free(buf), set_err("unterminated string"), NULL);
	s++;
	buf[len] = '\0';
	*out = buf;
	return (s);
}

static const char	*parse_value(const char *s, t_json **out);

static const char	*parse_array(const char *s, t_json **out)
{
	t_json	*arr;
	t_json	*item;

	arr = json_new(JSON_ARRAY);
	if (!arr)
		return (NULL);
	s = skip_ws(s + 1);
	if (*s == ']')
		return (*out = arr, s + 1);
	while (*s)
	{
		s = parse_value(s, &item);
		if (!s)
			return (json_free(arr), NULL);
		if (!arr_push(arr, item))
			return (json_free(item), json_free(arr), NULL);
		s = skip_ws(s);
		if (*s == ']')
			return (*out = arr, s + 1);
		if (*s != ',')
			return (json_free(arr), set_err("expected ',' or ']'"), NULL);
		s = skip_ws(s + 1);
	}
	return (json_free(arr), set_err("unterminated array"), NULL);
}

static int	obj_add_member(t_json *obj, char *key, t_json *value)
{
	t_json_member	*m;
	t_json_member	*cur;

	if (!obj || obj->type != JSON_OBJECT || !key)
		return (0);
	m = ft_calloc(1, sizeof(*m));
	if (!m)
		return (set_err("malloc failed"), 0);
	m->key = key;
	m->value = value;
	m->next = NULL;
	if (!obj->u.obj)
		obj->u.obj = m;
	else
	{
		cur = obj->u.obj;
		while (cur->next)
			cur = cur->next;
		cur->next = m;
	}
	return (1);
}

int	json_obj_set(t_json *obj, const char *key, t_json *value)
{
	t_json_member	*m;
	char			*dup;

	if (!obj || obj->type != JSON_OBJECT || !key || !value)
		return (0);
	m = obj->u.obj;
	while (m)
	{
		if (m->key && ft_strcmp(m->key, key) == 0)
		{
			json_free(m->value);
			m->value = value;
			return (1);
		}
		m = m->next;
	}
	dup = ft_strdup(key);
	if (!dup)
		return (set_err("malloc failed"), 0);
	if (!obj_add_member(obj, dup, value))
		return (free(dup), 0);
	return (1);
}

t_json	*json_new_null(void)
{
	return (json_new(JSON_NULL));
}

t_json	*json_new_bool(int v)
{
	t_json	*j;

	j = json_new(JSON_BOOL);
	if (!j)
		return (NULL);
	j->u.b = (v != 0);
	return (j);
}

t_json	*json_new_number(double v)
{
	t_json	*j;

	j = json_new(JSON_NUMBER);
	if (!j)
		return (NULL);
	j->u.n = v;
	return (j);
}

t_json	*json_new_string(const char *s)
{
	t_json	*j;

	if (!s)
		return (json_new(JSON_NULL));
	j = json_new(JSON_STRING);
	if (!j)
		return (NULL);
	j->u.s = ft_strdup(s);
	if (!j->u.s)
		return (json_free(j), set_err("malloc failed"), NULL);
	return (j);
}

t_json	*json_new_array(void)
{
	return (json_new(JSON_ARRAY));
}

t_json	*json_new_object(void)
{
	return (json_new(JSON_OBJECT));
}

static int	str_append(char **dst, size_t *len, const char *src)
{
	char	*tmp;
	size_t	slen;

	if (!dst || !len || !src)
		return (0);
	slen = ft_strlen(src);
	tmp = ft_calloc(*len + slen + 1, 1);
	if (!tmp)
		return (set_err("malloc failed"), 0);
	if (*dst)
		ft_memcpy(tmp, *dst, *len);
	ft_memcpy(tmp + *len, src, slen);
	free(*dst);
	*dst = tmp;
	*len += slen;
	return (1);
}

static int	str_append_n(char **dst, size_t *len, const char *src, size_t n)
{
	char	*tmp;

	if (!dst || !len || !src)
		return (0);
	tmp = ft_calloc(*len + n + 1, 1);
	if (!tmp)
		return (set_err("malloc failed"), 0);
	if (*dst)
		ft_memcpy(tmp, *dst, *len);
	ft_memcpy(tmp + *len, src, n);
	free(*dst);
	*dst = tmp;
	*len += n;
	return (1);
}

static int	append_indent(char **out, size_t *len, int pretty, int depth)
{
	int	i;

	if (!pretty)
		return (1);
	i = 0;
	while (i < depth)
	{
		if (!str_append(out, len, "\t"))
			return (0);
		i++;
	}
	return (1);
}

static int	append_escaped_string(char **out, size_t *len, const char *s)
{
	const char	*p;
	char		buf[2];

	if (!str_append(out, len, "\""))
		return (0);
	p = s;
	while (p && *p)
	{
		if (*p == '"')
		{
			if (!str_append(out, len, "\\\""))
				return (0);
		}
		else if (*p == '\\')
		{
			if (!str_append(out, len, "\\\\"))
				return (0);
		}
		else if (*p == '\n')
		{
			if (!str_append(out, len, "\\n"))
				return (0);
		}
		else if (*p == '\t')
		{
			if (!str_append(out, len, "\\t"))
				return (0);
		}
		else if (*p == '\r')
		{
			if (!str_append(out, len, "\\r"))
				return (0);
		}
		else
		{
			buf[0] = *p;
			buf[1] = '\0';
			if (!str_append(out, len, buf))
				return (0);
		}
		p++;
	}
	return (str_append(out, len, "\""));
}

static int	stringify_value(char **out, size_t *len, const t_json *j,
						int pretty, int depth);

static int	stringify_array(char **out, size_t *len, const t_json *j,
						int pretty, int depth)
{
	int	i;

	if (!str_append(out, len, "["))
		return (0);
	if (j->u.arr.len == 0)
		return (str_append(out, len, "]"));
	if (pretty && !str_append(out, len, "\n"))
		return (0);
	i = 0;
	while (i < j->u.arr.len)
	{
		if (!append_indent(out, len, pretty, depth + 1))
			return (0);
		if (!stringify_value(out, len, j->u.arr.items[i], pretty, depth + 1))
			return (0);
		i++;
		if (i < j->u.arr.len)
		{
			if (!str_append(out, len, ","))
				return (0);
		}
		if (pretty && !str_append(out, len, "\n"))
			return (0);
		if (!pretty && i < j->u.arr.len && !str_append(out, len, " "))
			return (0);
	}
	if (!append_indent(out, len, pretty, depth))
		return (0);
	return (str_append(out, len, "]"));
}

static int	stringify_object(char **out, size_t *len, const t_json *j,
						int pretty, int depth)
{
	t_json_member	*m;
	int			first;

	if (!str_append(out, len, "{"))
		return (0);
	if (!j->u.obj)
		return (str_append(out, len, "}"));
	if (pretty && !str_append(out, len, "\n"))
		return (0);
	m = j->u.obj;
	first = 1;
	while (m)
	{
		if (!first)
		{
			if (!str_append(out, len, ","))
				return (0);
			if (pretty && !str_append(out, len, "\n"))
				return (0);
			if (!pretty && !str_append(out, len, " "))
				return (0);
		}
		first = 0;
		if (!append_indent(out, len, pretty, depth + 1))
			return (0);
		if (!append_escaped_string(out, len, m->key))
			return (0);
		if (!str_append(out, len, pretty ? ": " : ":"))
			return (0);
		if (!stringify_value(out, len, m->value, pretty, depth + 1))
			return (0);
		m = m->next;
	}
	if (pretty && !str_append(out, len, "\n"))
		return (0);
	if (!append_indent(out, len, pretty, depth))
		return (0);
	return (str_append(out, len, "}"));
}

static int	stringify_value(char **out, size_t *len, const t_json *j,
						int pretty, int depth)
{
	char	buf[64];
	int	w;

	if (!j)
		return (str_append(out, len, "null"));
	if (j->type == JSON_NULL)
		return (str_append(out, len, "null"));
	if (j->type == JSON_BOOL)
		return (str_append(out, len, j->u.b ? "true" : "false"));
	if (j->type == JSON_NUMBER)
	{
		w = snprintf(buf, sizeof(buf), "%.17g", j->u.n);
		if (w < 0)
			return (0);
		return (str_append_n(out, len, buf, (size_t)w));
	}
	if (j->type == JSON_STRING)
		return (append_escaped_string(out, len, j->u.s ? j->u.s : ""));
	if (j->type == JSON_ARRAY)
		return (stringify_array(out, len, j, pretty, depth));
	if (j->type == JSON_OBJECT)
		return (stringify_object(out, len, j, pretty, depth));
	(void)pretty;
	(void)depth;
	return (0);
}

char	*json_stringify(const t_json *json, int pretty)
{
	char	*out;
	size_t	len;

	g_json_err = NULL;
	out = NULL;
	len = 0;
	if (!stringify_value(&out, &len, json, pretty != 0, 0))
		return (free(out), NULL);
	if (pretty && !str_append(&out, &len, "\n"))
		return (free(out), NULL);
	return (out);
}

int	json_write_file(const char *path, const t_json *json, int pretty)
{
	char	*s;
	int	fd;

	if (!path || *path == '\0' || !json)
		return (0);
	s = json_stringify(json, pretty);
	if (!s)
		return (0);
	fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd < 0)
		return (free(s), set_err("failed to open file for write"), 0);
	if (write(fd, s, ft_strlen(s)) < 0)
		return (close(fd), free(s), set_err("failed to write file"), 0);
	close(fd);
	free(s);
	return (1);
}

static const char	*parse_object(const char *s, t_json **out)
{
	t_json	*obj;
	char	*key;
	t_json	*value;

	obj = json_new(JSON_OBJECT);
	if (!obj)
		return (NULL);
	s = skip_ws(s + 1);
	if (*s == '}')
		return (*out = obj, s + 1);
	while (*s)
	{
		key = NULL;
		s = skip_ws(s);
		s = parse_str(s, &key);
		if (!s)
			return (json_free(obj), NULL);
		s = skip_ws(s);
		if (*s != ':')
			return (free(key), json_free(obj), set_err("expected ':'"), NULL);
		s = skip_ws(s + 1);
		s = parse_value(s, &value);
		if (!s)
			return (free(key), json_free(obj), NULL);
		if (!obj_add_member(obj, key, value))
			return (free(key), json_free(value), json_free(obj), NULL);
		s = skip_ws(s);
		if (*s == '}')
			return (*out = obj, s + 1);
		if (*s != ',')
			return (json_free(obj), set_err("expected ',' or '}'"), NULL);
		s = skip_ws(s + 1);
	}
	return (json_free(obj), set_err("unterminated object"), NULL);
}

static const char	*parse_number(const char *s, t_json **out)
{
	t_json	*j;
	char	*end;
	double	n;

	errno = 0;
	n = strtod(s, &end);
	if (end == s)
		return (set_err("invalid number"), NULL);
	if (errno == ERANGE)
		return (set_err("number out of range"), NULL);
	j = json_new(JSON_NUMBER);
	if (!j)
		return (NULL);
	j->u.n = n;
	*out = j;
	return (end);
}

static int	match_kw(const char *s, const char *kw)
{
	int	i;

	i = 0;
	while (kw[i])
	{
		if (s[i] != kw[i])
			return (0);
		i++;
	}
	return (1);
}

static const char	*parse_value(const char *s, t_json **out)
{
	t_json	*j;
	char	*str;

	s = skip_ws(s);
	if (!*s)
		return (set_err("unexpected end of input"), NULL);
	if (*s == '"')
	{
		str = NULL;
		s = parse_str(s, &str);
		if (!s)
			return (NULL);
		j = json_new(JSON_STRING);
		if (!j)
			return (free(str), NULL);
		j->u.s = str;
		*out = j;
		return (s);
	}
	if (*s == '{')
		return (parse_object(s, out));
	if (*s == '[')
		return (parse_array(s, out));
	if (*s == '-' || (*s >= '0' && *s <= '9'))
		return (parse_number(s, out));
	if (match_kw(s, "true"))
	{
		j = json_new(JSON_BOOL);
		if (!j)
			return (NULL);
		j->u.b = 1;
		*out = j;
		return (s + 4);
	}
	if (match_kw(s, "false"))
	{
		j = json_new(JSON_BOOL);
		if (!j)
			return (NULL);
		j->u.b = 0;
		*out = j;
		return (s + 5);
	}
	if (match_kw(s, "null"))
	{
		*out = json_new(JSON_NULL);
		if (!*out)
			return (NULL);
		return (s + 4);
	}
	return (set_err("invalid value"), NULL);
}

t_json	*json_parse_str(const char *src)
{
	t_json		*j;
	const char	*s;

	g_json_err = NULL;
	if (!src)
		return (set_err("null input"), NULL);
	s = parse_value(src, &j);
	if (!s)
		return (NULL);
	s = skip_ws(s);
	if (*s != '\0')
		return (json_free(j), set_err("trailing content"), NULL);
	return (j);
}

static char	*read_all(const char *path)
{
	int		fd;
	char	buf[4096];
	ssize_t	br;
	char	*out;
	char	*tmp;
	size_t	len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (set_err("failed to open file"), NULL);
	out = ft_strdup("");
	if (!out)
		return (close(fd), set_err("malloc failed"), NULL);
	len = 0;
	br = 1;
	while (br > 0)
	{
		br = read(fd, buf, sizeof(buf));
		if (br < 0)
			return (free(out), close(fd), set_err("failed to read file"), NULL);
		if (br == 0)
			break ;
		tmp = ft_calloc(len + (size_t)br + 1, 1);
		if (!tmp)
			return (free(out), close(fd), set_err("malloc failed"), NULL);
		ft_memcpy(tmp, out, len);
		ft_memcpy(tmp + len, buf, (size_t)br);
		free(out);
		out = tmp;
		len += (size_t)br;
	}
	close(fd);
	return (out);
}

t_json	*json_parse_file(const char *path)
{
	char	*src;
	t_json	*j;

	g_json_err = NULL;
	if (!path || *path == '\0')
		return (set_err("invalid path"), NULL);
	src = read_all(path);
	if (!src)
		return (NULL);
	j = json_parse_str(src);
	free(src);
	return (j);
}
