/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   editor_main.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cadete <cadete@student.42sp.org.br>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 00:00:00 by cadete            #+#    #+#             */
/*   Updated: 2026/04/01 00:00:00 by cadete           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <MLX42/MLX42.h>

#include <cstring>
#include <cstdio>
#include <cstdarg>

#include <vector>
#include <string>

#include <dirent.h>
#include <sys/stat.h>

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "cimgui.h"

extern "C" {
#include "cimcolortextedit.h"
}

extern "C" {
#include "editor/editor_runtime.h"
#include "engine/engine.h"
}

typedef struct s_editor
{
	mlx_t				*mlx;
	t_editor_runtime	rt;
	GLuint				preview_tex;
	int					preview_w;
	int					preview_h;
	uint32_t			selected_entity;
	std::vector<std::string>	logs;
	char				project_path[256];
	TextEditor			*script_editor;
	std::string			script_open_path;
} 	t_editor;

static bool	ends_with(const std::string &s, const char *suffix)
{
	std::size_t	ls;
	std::size_t	lf;

	if (!suffix)
		return (false);
	lf = std::strlen(suffix);
	ls = s.size();
	if (lf > ls)
		return (false);
	return (s.compare(ls - lf, lf, suffix) == 0);
}

static bool	read_file_text(const char *path, std::string &out)
{
	FILE	*fp;
	long	sz;
	char	*buf;

	out.clear();
	if (!path)
		return (false);
	fp = std::fopen(path, "rb");
	if (!fp)
		return (false);
	if (std::fseek(fp, 0, SEEK_END) != 0)
		return (std::fclose(fp), false);
	sz = std::ftell(fp);
	if (sz < 0)
		return (std::fclose(fp), false);
	if (std::fseek(fp, 0, SEEK_SET) != 0)
		return (std::fclose(fp), false);
	buf = (char *)std::malloc((size_t)sz + 1);
	if (!buf)
		return (std::fclose(fp), false);
	if (sz > 0 && std::fread(buf, 1, (size_t)sz, fp) != (size_t)sz)
		return (std::free(buf), std::fclose(fp), false);
	buf[sz] = '\0';
	out.assign(buf);
	std::free(buf);
	std::fclose(fp);
	return (true);
}

static bool	write_file_text(const char *path, const char *text)
{
	FILE	*fp;
	size_t	len;

	if (!path || !text)
		return (false);
	fp = std::fopen(path, "wb");
	if (!fp)
		return (false);
	len = std::strlen(text);
	if (len > 0 && std::fwrite(text, 1, len, fp) != len)
		return (std::fclose(fp), false);
	std::fclose(fp);
	return (true);
}

static ImVec2_c	v2(float x, float y)
{
	ImVec2_c	v;

	v.x = x;
	v.y = y;
	return (v);
}

static void	logf(t_editor *ed, const char *fmt, ...)
{
	char	buf[1024];
	va_list	ap;
	int		n;

	if (!ed || !fmt)
		return ;
	va_start(ap, fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n < 0)
		return ;
	ed->logs.push_back(std::string(buf));
}

static void	gl_tex_init(t_editor *ed, int w, int h)
{
	if (!ed)
		return ;
	ed->preview_w = w;
	ed->preview_h = h;
	if (ed->preview_tex != 0)
		glDeleteTextures(1, &ed->preview_tex);
	glGenTextures(1, &ed->preview_tex);
	glBindTexture(GL_TEXTURE_2D, ed->preview_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);
}

static void	gl_tex_update(t_editor *ed, const mlx_image_t *img)
{
	if (!ed || !img || ed->preview_tex == 0)
		return ;
	if ((int)img->width != ed->preview_w || (int)img->height != ed->preview_h)
		gl_tex_init(ed, (int)img->width, (int)img->height);
	glBindTexture(GL_TEXTURE_2D, ed->preview_tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ed->preview_w, ed->preview_h,
		GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
}

static void	ensure_default_scene_path(t_editor *ed, t_engine *engine)
{
	struct stat	st;
	const char	*base;
	const char	*slash;
	std::string	name;
	std::string	scene;
	std::size_t	dot;

	if (!ed || !engine || engine->scene_path || !engine->project_path)
		return ;
	if (stat("scenes", &st) != 0)
		mkdir("scenes", 0755);
	slash = std::strrchr(engine->project_path, '/');
	base = slash ? slash + 1 : engine->project_path;
	name = std::string(base);
	dot = name.rfind('.');
	if (dot != std::string::npos)
		name = name.substr(0, dot);
	scene = std::string("scenes/") + name + ".scene.json";
	engine->scene_path = strdup(scene.c_str());
	if (engine->scene_path)
		logf(ed, "scene: %s", engine->scene_path);
}

static void	ui_toolbar_bar(t_editor *ed)
{
	bool		play;
	bool		want_save;
	bool		want_reload;
	bool		want_export;
	t_engine	*engine;

	engine = editor_runtime_engine(&ed->rt);
	play = (ed->rt.play_mode != 0);
	want_save = false;
	want_reload = false;
	want_export = false;
	if (ImGui::Button(play ? "Stop" : "Run"))
	{
		editor_runtime_set_play_mode(&ed->rt, !play);
		logf(ed, "play_mode=%s", !play ? "on" : "off");
	}
	ImGui::SameLine(0.0f, 8.0f);
	want_save = ImGui::Button("Save");
	ImGui::SameLine(0.0f, 8.0f);
	want_reload = ImGui::Button("Reload scripts");
	ImGui::SameLine(0.0f, 8.0f);
	want_export = ImGui::Button("Build/Export");
	if (engine && want_save)
	{
		ensure_default_scene_path(ed, engine);
		if (!engine_save_scene(engine) || !engine_save_project(engine))
			logf(ed, "save failed");
		else
			logf(ed, "saved: %s", engine->project_path);
	}
	if (engine && want_reload)
	{
		if (!engine_reload_scripts(engine))
			logf(ed, "reload scripts failed");
		else
			logf(ed, "scripts reloaded");
	}
	if (want_export)
	{
		int	r;
		logf(ed, "export: running make && staging dist/");
		r = system("make -j4 >/dev/null 2>&1");
		if (r != 0)
			logf(ed, "export: build failed");
		else
		{
			system("rm -rf dist && mkdir -p dist");
			system("cp -f cub3D dist/cub3D");
			system("cp -rf assets scripts prefabs maps scenes dist/ >/dev/null 2>&1 || true");
			logf(ed, "export: done (dist/)");
		}
	}
}

static void	ui_dockspace(t_editor *ed)
{
	ImGuiWindowFlags	flags;
	ImGuiViewport		*vp;
	ImGuiID				dockspace_id;
	static bool			built = false;

	(void)ed;
	flags = ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_MenuBar;
	vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::SetNextWindowViewport(vp->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin("DockSpaceHost", NULL, flags);
	ImGui::PopStyleVar(2);
	if (ImGui::BeginMenuBar())
	{
		ui_toolbar_bar(ed);
		ImGui::EndMenuBar();
	}
	dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0),
		ImGuiDockNodeFlags_PassthruCentralNode);
	if (!built)
	{
		ImGuiID	left;
		ImGuiID	right;
		ImGuiID	bottom;
		ImGuiID	center;

		built = true;
		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id,
			ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);
		center = dockspace_id;
		left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f,
			NULL, &center);
		right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.26f,
			NULL, &center);
		bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f,
			NULL, &center);
		ImGui::DockBuilderDockWindow("Hierarchy", left);
		ImGui::DockBuilderDockWindow("Inspector", right);
		ImGui::DockBuilderDockWindow("Scene", center);
		ImGui::DockBuilderDockWindow("Scene View", center);
		ImGui::DockBuilderDockWindow("Script Editor", center);
		ImGui::DockBuilderDockWindow("Content Drawer", bottom);
		ImGui::DockBuilderDockWindow("Console", bottom);
		ImGui::DockBuilderDockWindow("File System", bottom);
		ImGui::DockBuilderFinish(dockspace_id);
	}
	ImGui::End();
}

static void	ui_preview(t_editor *ed)
{
	ImVec2	avail;
	float	scale;
	float	w;
	float	h;

	igBegin("Scene", NULL, 0);
	avail = ImGui::GetContentRegionAvail();
	if (ed->preview_tex != 0 && ed->preview_w > 0 && ed->preview_h > 0)
	{
		ImTextureRef_c	tex;

		tex._TexData = NULL;
		tex._TexID = (ImTextureID)(intptr_t)ed->preview_tex;
		scale = 1.0f;
		w = (float)ed->preview_w;
		h = (float)ed->preview_h;
		if (w > 0.0f && h > 0.0f)
		{
			scale = avail.x / w;
			if (h * scale > avail.y)
				scale = avail.y / h;
			if (scale <= 0.0f)
				scale = 1.0f;
		}
		igImage(tex,
			v2(w * scale, h * scale),
			v2(0, 0), v2(1, 1));
	}
	else
		igText("preview not ready");
	igEnd();
}

static void	toggle_cell(t_engine *engine, int x, int y)
{
	char	*c;
	int		len;

	if (!engine || !engine->file.map || y < 0)
		return ;
	if (!engine->file.map[y])
		return ;
	len = (int)std::strlen(engine->file.map[y]);
	if (x < 0 || x >= len)
		return ;
	c = &engine->file.map[y][x];
	if (*c == '1')
		*c = '0';
	else if (*c == '0')
		*c = '1';
}

static void	ui_scene_view(t_editor *ed)
{
	t_engine	*engine;
	const float	cell = 14.0f;
	ImVec2		origin;
	ImDrawList	*dl;
	int			y;
	int			x;
	int			w;
	int			h;
	ImVec2		mouse;
	bool		hover;

	engine = editor_runtime_engine(&ed->rt);
	igBegin("Scene View", NULL, 0);
	if (!engine || !engine->file.map)
	{
		igText("no map loaded");
		igEnd();
		return ;
	}
	igText("Click cells to toggle 0/1 (floor/wall)");
	igSeparator();
	origin = ImGui::GetCursorScreenPos();
	dl = ImGui::GetWindowDrawList();
	h = engine->file.map_height;
	w = engine->file.map_width;
	if (w <= 0 || h <= 0)
		return (igEnd());
	ImGui::InvisibleButton("map_canvas", ImVec2(w * cell, h * cell));
	hover = ImGui::IsItemHovered();
	mouse = ImGui::GetMousePos();
	y = 0;
	while (y < h)
	{
		x = 0;
		while (x < w)
		{
			char	v = ' ';
			int		line_len;
			ImU32	col;
			ImVec2	p0(origin.x + x * cell, origin.y + y * cell);
			ImVec2	p1(p0.x + cell - 1.0f, p0.y + cell - 1.0f);
			line_len = (int)std::strlen(engine->file.map[y]);
			if (x < line_len)
				v = engine->file.map[y][x];
			col = IM_COL32(40, 40, 40, 255);
			if (engine->app.spawn_x == x && engine->app.spawn_y == y)
				col = IM_COL32(20, 80, 140, 255);
			if (v == '1')
				col = IM_COL32(120, 120, 120, 255);
			else if (v == '0')
				col = IM_COL32(20, 20, 20, 255);
			dl->AddRectFilled(p0, p1, col);
			dl->AddRect(p0, p1, IM_COL32(60, 60, 60, 255));
			x++;
		}
		y++;
	}
	if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		int gx = (int)((mouse.x - origin.x) / cell);
		int gy = (int)((mouse.y - origin.y) / cell);
		if (gx != engine->app.spawn_x || gy != engine->app.spawn_y)
			toggle_cell(engine, gx, gy);
	}
	igEnd();
}

static void	ui_entities(t_editor *ed)
{
	t_engine	*engine;
	uint32_t	id;
	t_entity	*e;
	char		label[128];

	engine = editor_runtime_engine(&ed->rt);
	igBegin("Hierarchy", NULL, 0);
	if (!engine)
		return (igEnd());
	id = 1;
	while (id < engine->scene.store.next_id)
	{
		e = entity_get(&engine->scene.store, id);
		if (e)
		{
			snprintf(label, sizeof(label), "%u: %s", e->id,
				e->type ? e->type : "(null)");
			if (igSelectable_Bool(label, ed->selected_entity == e->id, 0,
				v2(0, 0)))
				ed->selected_entity = e->id;
		}
		id++;
	}
	igEnd();
}

static void	ui_inspector(t_editor *ed)
{
	t_engine	*engine;
	t_entity	*e;
	t_property	*p;
	char		buf[256];
	bool		b;

	engine = editor_runtime_engine(&ed->rt);
	igBegin("Inspector", NULL, 0);
	if (!engine)
		return (igEnd());
	igText("Project");
	{
		float	floor_col[3];
		float	ceil_col[3];

		floor_col[0] = (float)engine->app.floor.r / 255.0f;
		floor_col[1] = (float)engine->app.floor.g / 255.0f;
		floor_col[2] = (float)engine->app.floor.b / 255.0f;
		ceil_col[0] = (float)engine->app.ceiling.r / 255.0f;
		ceil_col[1] = (float)engine->app.ceiling.g / 255.0f;
		ceil_col[2] = (float)engine->app.ceiling.b / 255.0f;
		if (igColorEdit3("floor", floor_col, 0))
		{
			engine->app.floor.r = (int)(floor_col[0] * 255.0f + 0.5f);
			engine->app.floor.g = (int)(floor_col[1] * 255.0f + 0.5f);
			engine->app.floor.b = (int)(floor_col[2] * 255.0f + 0.5f);
		}
		if (igColorEdit3("ceiling", ceil_col, 0))
		{
			engine->app.ceiling.r = (int)(ceil_col[0] * 255.0f + 0.5f);
			engine->app.ceiling.g = (int)(ceil_col[1] * 255.0f + 0.5f);
			engine->app.ceiling.b = (int)(ceil_col[2] * 255.0f + 0.5f);
		}
	}
	igSeparator();
	e = entity_get(&engine->scene.store, ed->selected_entity);
	if (!e)
		return (igText("select an entity"), igEnd());
	igText("id: %u", e->id);
	igText("type: %s", e->type ? e->type : "(null)");
	if (e->script_path)
		igText("script: %s", e->script_path);
	igSeparator();
	{
		float	x = (float)e->transform.x;
		float	y = (float)e->transform.y;
		float	r = (float)e->transform.rot;
		if (igDragFloat("x", &x, 0.01f, -10000.0f, 10000.0f, "%.3f", 0))
			e->transform.x = (double)x;
		if (igDragFloat("y", &y, 0.01f, -10000.0f, 10000.0f, "%.3f", 0))
			e->transform.y = (double)y;
		if (igDragFloat("rot", &r, 0.01f, -10000.0f, 10000.0f, "%.3f", 0))
			e->transform.rot = (double)r;
	}
	igSeparator();
	p = e->properties;
	while (p)
	{
		if (p->key && p->key[0] == '_' && p->key[1] == '_')
		{
			p = p->next;
			continue ;
		}
		if (p->type == PROP_NUMBER)
		{
			float f = (float)p->v.n;
			if (igDragFloat(p->key, &f, 0.05f, -100000.0f, 100000.0f, "%.3f", 0))
				p->v.n = (double)f;
		}
		else if (p->type == PROP_BOOL)
		{
			b = (p->v.b != 0);
			if (igCheckbox(p->key, &b))
				p->v.b = b ? 1 : 0;
		}
		else if (p->type == PROP_STRING)
		{
			std::snprintf(buf, sizeof(buf), "%s", p->v.s ? p->v.s : "");
			if (igInputText(p->key, buf, sizeof(buf), 0, NULL, NULL))
			{
				free(p->v.s);
				p->v.s = strdup(buf);
			}
		}
		p = p->next;
	}
	igEnd();
}

static void	list_dir_simple(const char *path, std::vector<std::string> &out)
{
	DIR			*dir;
	struct dirent	*ent;

	dir = opendir(path);
	if (!dir)
		return ;
	while ((ent = readdir(dir)) != NULL)
	{
		if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0)
			continue ;
		out.push_back(std::string(ent->d_name));
	}
	closedir(dir);
}

static void	ui_assets(t_editor *ed)
{
	std::vector<std::string>	items;
	int					i;

	(void)ed;
	igBegin("Content Drawer", NULL, 0);
	items.clear();
	list_dir_simple("assets", items);
	igText("assets/");
	i = 0;
	while (i < (int)items.size())
	{
		igBulletText("%s", items[i].c_str());
		i++;
	}
	items.clear();
	list_dir_simple("prefabs", items);
	igSeparator();
	igText("prefabs/");
	i = 0;
	while (i < (int)items.size())
	{
		igBulletText("%s", items[i].c_str());
		i++;
	}
	items.clear();
	list_dir_simple("scripts", items);
	igSeparator();
	igText("scripts/");
	i = 0;
	while (i < (int)items.size())
	{
		igBulletText("%s", items[i].c_str());
		i++;
	}
	igEnd();
}

static void	ui_script_editor(t_editor *ed)
{
	std::vector<std::string>	items;
	int						i;
	std::string					path;
	std::string					content;
	ImVec2						avail;
	bool						want_save;
	bool						want_reload;

	if (!ed)
		return ;
	igBegin("Script Editor", NULL, 0);
	if (!ed->script_editor)
		ed->script_editor = ImColorTextEdit_TextEditor();
	want_save = igButton("Save", v2(0, 0));
	igSameLine(0.0f, 8.0f);
	want_reload = igButton("Reload", v2(0, 0));
	igSeparator();
	igBeginChild_Str("##scripts", v2(220, 0), true, 0);
	items.clear();
	list_dir_simple("scripts", items);
	i = 0;
	while (i < (int)items.size())
	{
		if (!ends_with(items[i], ".lua"))
		{
			i++;
			continue ;
		}
		if (igSelectable_Bool(items[i].c_str(),
			ed->script_open_path == (std::string("scripts/") + items[i]), 0, v2(0, 0)))
		{
			path = std::string("scripts/") + items[i];
			if (read_file_text(path.c_str(), content))
			{
				ed->script_open_path = path;
				ImColorTextEdit_TextEditor_SetText(ed->script_editor, content.c_str());
				logf(ed, "script opened: %s", path.c_str());
			}
			else
				logf(ed, "script open failed: %s", path.c_str());
		}
		i++;
	}
	igEndChild();
	igSameLine(0.0f, 8.0f);
	igBeginGroup();
	if (ed->script_open_path.empty())
		igText("Select a script in scripts/");
	else
	{
		igText("Editing: %s", ed->script_open_path.c_str());
		avail = ImGui::GetContentRegionAvail();
		ImColorTextEdit_TextEditor_RenderSizeBorder(ed->script_editor,
			"##cte", &avail, true);
		if (want_reload)
		{
			if (read_file_text(ed->script_open_path.c_str(), content))
			{
				ImColorTextEdit_TextEditor_SetText(ed->script_editor, content.c_str());
				logf(ed, "script reloaded: %s", ed->script_open_path.c_str());
			}
			else
				logf(ed, "script reload failed: %s", ed->script_open_path.c_str());
		}
		if (want_save)
		{
			const char	*txt = ImColorTextEdit_TextEditor_GetText(ed->script_editor);
			if (txt && write_file_text(ed->script_open_path.c_str(), txt))
				logf(ed, "script saved: %s", ed->script_open_path.c_str());
			else
				logf(ed, "script save failed: %s", ed->script_open_path.c_str());
		}
	}
	igEndGroup();
	igEnd();
}

static void	uifs_node(const char *path)
{
	DIR			*dir;
	struct dirent	*ent;
	struct stat	st;
	std::string	child;

	dir = opendir(path);
	if (!dir)
		return ;
	while ((ent = readdir(dir)) != NULL)
	{
		if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0)
			continue ;
		child = std::string(path) + "/" + ent->d_name;
		if (stat(child.c_str(), &st) != 0)
			continue ;
		if (S_ISDIR(st.st_mode))
		{
			if (igTreeNode_Str(ent->d_name))
			{
				uifs_node(child.c_str());
				igTreePop();
			}
		}
		else
			igBulletText("%s", ent->d_name);
	}
	closedir(dir);
}

static void	ui_filesystem(t_editor *ed)
{
	(void)ed;
	igBegin("File System", NULL, 0);
	uifs_node(".");
	igEnd();
}

static void	ui_console(t_editor *ed)
{
	int	i;

	igBegin("Console", NULL, 0);
	i = 0;
	while (i < (int)ed->logs.size())
	{
		igTextUnformatted(ed->logs[i].c_str(), NULL);
		i++;
	}
	igEnd();
}

static void	editor_frame(void *param)
{
	t_editor	*ed;
	mlx_image_t	*img;
	double		dt;
	char		line[512];

	ed = (t_editor *)param;
	dt = ed->mlx->delta_time;
	editor_runtime_tick(&ed->rt, dt);
	while (editor_runtime_log_pop(&ed->rt, line, (int)sizeof(line)))
		ed->logs.push_back(std::string(line));
	editor_runtime_render(&ed->rt);
	img = editor_runtime_frame_image(&ed->rt);
	if (img)
		gl_tex_update(ed, img);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	igNewFrame();
	ui_dockspace(ed);
	ui_preview(ed);
	ui_scene_view(ed);
	ui_entities(ed);
	ui_inspector(ed);
	ui_assets(ed);
	ui_script_editor(ed);
	ui_filesystem(ed);
	ui_console(ed);
	igRender();
	ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

static int	editor_init(t_editor *ed)
{
	ImGuiContext	*ctx;
	GLFWwindow		*win;
	mlx_image_t		*img;

	ed->mlx = NULL;
	ed->preview_tex = 0;
	ed->preview_w = 0;
	ed->preview_h = 0;
	ed->selected_entity = 0;
	ed->logs.clear();
	ed->script_editor = NULL;
	ed->script_open_path.clear();
	std::snprintf(ed->project_path, sizeof(ed->project_path), "%s",
		"maps/level01.json");
	ed->mlx = mlx_init(1280, 720, "Raycast Editor", true);
	if (!ed->mlx)
		return (0);
	ctx = igCreateContext(NULL);
	if (!ctx)
		return (0);
	{
		ImGuiIO	&io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}
	win = (GLFWwindow *)ed->mlx->window;
	if (!ImGui_ImplGlfw_InitForOpenGL(win, true))
		return (0);
	if (!ImGui_ImplOpenGL3_Init("#version 130"))
		return (0);
	if (!editor_runtime_init(&ed->rt, ed->mlx, ed->project_path))
		return (0);
	img = editor_runtime_frame_image(&ed->rt);
	if (img)
		gl_tex_init(ed, (int)img->width, (int)img->height);
	logf(ed, "loaded project: %s", ed->project_path);
	mlx_loop_hook(ed->mlx, editor_frame, ed);
	return (1);
}

static void	editor_destroy(t_editor *ed)
{
	if (!ed)
		return ;
	editor_runtime_destroy(&ed->rt);
	if (ed->script_editor)
		ImColorTextEdit_destroy(ed->script_editor);
	ed->script_editor = NULL;
	ed->script_open_path.clear();
	if (ed->preview_tex != 0)
		glDeleteTextures(1, &ed->preview_tex);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	igDestroyContext(NULL);
	if (ed->mlx)
		mlx_terminate(ed->mlx);
	ed->mlx = NULL;
}

int	main(void)
{
	t_editor	ed;

	if (!editor_init(&ed))
	{
		std::fprintf(stderr, "error: failed to init editor\n");
		return (1);
	}
	mlx_loop(ed.mlx);
	editor_destroy(&ed);
	return (0);
}
