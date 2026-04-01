NAME		= cub3D
EDITOR_NAME	= editor
EDITOR_BIN	= $(BIN_DIR)/$(EDITOR_NAME)
CC			= cc
CXX			= c++
GCC			= gcc-10
CFLAGS		= -Wall -Wextra -Werror -g
CXXFLAGS	= -Wall -Wextra -Werror -g -std=c++11 -fno-exceptions -fno-rtti
VENDOR_CXXFLAGS	= -Wall -Wextra -g -std=c++11 -fno-exceptions -fno-rtti \
			-Wno-reorder -Wno-sign-compare -Wno-unused-variable -Wno-sequence-point
RM			= rm -rf

SRC_DIR		= src
BUILD_DIR	= build
OBJ_DIR		= $(BUILD_DIR)/obj
BIN_DIR		= $(BUILD_DIR)/bin

LIBFT_DIR	= lib/libft
LIBFT_LIB	= ${LIBFT_DIR}/libft.a
MLX42_DIR	= lib/MLX42
MLX42_BUILD	= $(MLX42_DIR)/build
MLX42_LIB	= $(MLX42_BUILD)/libmlx42.a

XPM_CONV	= python3 tools/xpm_to_xpm42.py
XPM_FILES	= $(wildcard assets/textures/*.xpm)
XPM42_FILES	= $(XPM_FILES:.xpm=.xpm42)

CRIT_DIR	= lib/criterion-2.4.3
CRIT_INC	= -I $(CRIT_DIR)/include
CRIT_LIBDIR	= $(CRIT_DIR)/lib
CRIT_LIBS	= -L $(CRIT_LIBDIR) -lcriterion

INCLUDE		= -I include -I ${LIBFT_DIR} -I $(MLX42_DIR)/include
LUA_CFLAGS	= $(shell pkg-config --cflags lua5.4 2>/dev/null)
LUA_LIBS	= $(shell pkg-config --libs lua5.4 2>/dev/null)

INCLUDE		+= $(LUA_CFLAGS)
LDFLAGS		= -ldl -lglfw -pthread -lm $(LUA_LIBS)

IMGUI_DIR	= lib/cimgui
IMGUI_INC	= -I $(IMGUI_DIR) -I $(IMGUI_DIR)/imgui -I $(IMGUI_DIR)/imgui/backends
IMGUI_DEFS	= -DCIMGUI_USE_GLFW -DCIMGUI_USE_OPENGL3 -DIMGUI_IMPL_OPENGL_LOADER_GLAD

IMTEXT_DIR	= lib/cimgui_colortextedit
IMTEXT_INC	= -I $(IMTEXT_DIR) -I $(IMTEXT_DIR)/ImGuiColorTextEdit

SRCS		= $(SRC_DIR)/main.c \
			  engine/engine.c \
			  engine/core/json.c \
			  engine/scene/entity.c \
			  engine/scene/scene.c \
			  engine/scene/scene_io.c \
			  engine/scene/prefab.c \
			  engine/scripting/lua_engine.c \
			  $(SRC_DIR)/events/close.c \
			  $(SRC_DIR)/events/input.c \
			  $(SRC_DIR)/init/framebuffer.c \
			  $(SRC_DIR)/init/textures_load.c \
			  $(SRC_DIR)/render/render.c \
			  $(SRC_DIR)/render/fps.c \
			  $(SRC_DIR)/render/wall_slices.c \
			  $(SRC_DIR)/raycasting/raycast_core.c \
			  $(SRC_DIR)/raycasting/raycast_init.c \
			  $(SRC_DIR)/raycasting/raycast_dda.c \
			  $(SRC_DIR)/player/player_init.c \
			  $(SRC_DIR)/player/rotate.c \
			  $(SRC_DIR)/player/movement.c \
			  $(SRC_DIR)/player/movement_collision.c \
			  $(SRC_DIR)/player/debug_term.c \
			  $(SRC_DIR)/parsing/file.c \
			  $(SRC_DIR)/parsing/utils.c \
			  $(SRC_DIR)/parsing/textures.c \
			  $(SRC_DIR)/parsing/colors.c \
			  $(SRC_DIR)/parsing/colors_utils.c \
			  $(SRC_DIR)/parsing/map_parse.c \
			  $(SRC_DIR)/parsing/map_parse_utils.c \
			  $(SRC_DIR)/parsing/map_closed.c \
			  $(SRC_DIR)/parsing/map_closed_utils.c \
			  $(SRC_DIR)/parsing/player_spawn.c
OBJS		= ${SRCS:%.c=${OBJ_DIR}/%.o}

EDITOR_C_SRCS	= $(SRC_DIR)/editor_main.cpp

EDITOR_EMBED_SRCS	= $(SRC_DIR)/editor/editor_runtime.c \
			  engine/engine.c \
			  engine/core/json.c \
			  engine/scene/entity.c \
			  engine/scene/scene.c \
			  engine/scene/scene_io.c \
			  engine/scene/prefab.c \
			  engine/scripting/lua_engine.c \
			  $(SRC_DIR)/events/close.c \
			  $(SRC_DIR)/init/framebuffer.c \
			  $(SRC_DIR)/init/textures_load.c \
			  $(SRC_DIR)/render/render.c \
			  $(SRC_DIR)/render/fps.c \
			  $(SRC_DIR)/render/wall_slices.c \
			  $(SRC_DIR)/raycasting/raycast_core.c \
			  $(SRC_DIR)/raycasting/raycast_init.c \
			  $(SRC_DIR)/raycasting/raycast_dda.c \
			  $(SRC_DIR)/player/player_init.c \
			  $(SRC_DIR)/player/rotate.c \
			  $(SRC_DIR)/player/movement.c \
			  $(SRC_DIR)/player/movement_collision.c \
			  $(SRC_DIR)/player/debug_term.c \
			  $(SRC_DIR)/parsing/file.c \
			  $(SRC_DIR)/parsing/utils.c \
			  $(SRC_DIR)/parsing/textures.c \
			  $(SRC_DIR)/parsing/colors.c \
			  $(SRC_DIR)/parsing/colors_utils.c \
			  $(SRC_DIR)/parsing/map_parse.c \
			  $(SRC_DIR)/parsing/map_parse_utils.c \
			  $(SRC_DIR)/parsing/map_closed.c \
			  $(SRC_DIR)/parsing/map_closed_utils.c \
			  $(SRC_DIR)/parsing/player_spawn.c
EDITOR_CPP_SRCS	= $(IMGUI_DIR)/cimgui.cpp \
			  $(IMGUI_DIR)/imgui/imgui.cpp \
			  $(IMGUI_DIR)/imgui/imgui_draw.cpp \
			  $(IMGUI_DIR)/imgui/imgui_demo.cpp \
			  $(IMGUI_DIR)/imgui/imgui_tables.cpp \
			  $(IMGUI_DIR)/imgui/imgui_widgets.cpp \
			  $(IMGUI_DIR)/imgui/backends/imgui_impl_glfw.cpp \
			  $(IMGUI_DIR)/imgui/backends/imgui_impl_opengl3.cpp \
			  $(IMTEXT_DIR)/ImGuiColorTextEdit/TextEditor.cpp \
			  $(IMTEXT_DIR)/cimcolortextedit.cpp

EDITOR_OBJS		= $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(EDITOR_C_SRCS)) \
			  $(patsubst %.c,$(OBJ_DIR)/%.o,$(EDITOR_EMBED_SRCS)) \
			  $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(EDITOR_CPP_SRCS))

TEST_NAME	= tests_bin
TEST_DIR	= tests
TEST_SRCS	= $(TEST_DIR)/unit/parsing/test_parsing_unit.c \
			  $(TEST_DIR)/integration/parsing/test_parsing_integration.c \
			  $(SRC_DIR)/parsing/file.c \
			  $(SRC_DIR)/parsing/utils.c \
			  $(SRC_DIR)/parsing/textures.c \
			  $(SRC_DIR)/parsing/colors.c \
			  $(SRC_DIR)/parsing/colors_utils.c \
			  $(SRC_DIR)/parsing/map_parse.c \
			  $(SRC_DIR)/parsing/map_parse_utils.c \
			  $(SRC_DIR)/parsing/map_closed.c \
			  $(SRC_DIR)/parsing/map_closed_utils.c \
			  $(SRC_DIR)/parsing/player_spawn.c
TEST_OBJS	= ${TEST_SRCS:%.c=${OBJ_DIR}/%.test.o}

# Coverage flags (tests only)
COV_FLAGS := -fprofile-arcs -ftest-coverage -O0 -g

GCOVR		= ./lib/gcovr-8.6-linux-x86_64
GCOV		= gcov-10

# ======================== BINARY =========================
all: ${NAME}

editor: $(EDITOR_BIN)
	cp $(EDITOR_BIN) $(EDITOR_NAME)

${NAME}: ${XPM42_FILES} ${OBJS} ${LIBFT_LIB} ${MLX42_LIB}
	mkdir -p ${BIN_DIR}
	${CC} ${CFLAGS} -no-pie ${OBJS} ${LIBFT_LIB} ${MLX42_LIB} ${LDFLAGS} -o ${BIN_DIR}/${NAME}
	cp ${BIN_DIR}/${NAME} ${NAME}

$(EDITOR_BIN): ${EDITOR_OBJS} ${LIBFT_LIB} ${MLX42_LIB}
	mkdir -p ${BIN_DIR}
	${CXX} ${CXXFLAGS} -no-pie ${EDITOR_OBJS} ${LIBFT_LIB} ${MLX42_LIB} ${LDFLAGS} -lGL -o $(EDITOR_BIN)

assets/textures/%.xpm42: assets/textures/%.xpm
	$(XPM_CONV) $<

${OBJ_DIR}/%.o: %.c
	mkdir -p ${@D}
	${CC} ${CFLAGS} ${INCLUDE} -c $< -o $@

${OBJ_DIR}/%.o: %.cpp
	mkdir -p ${@D}
	${CXX} ${CXXFLAGS} ${INCLUDE} $(IMGUI_INC) $(IMTEXT_INC) $(IMGUI_DEFS) -c $< -o $@

# Vendor libs: compile without -Werror (upstream warnings)
${OBJ_DIR}/lib/cimgui_colortextedit/%.o: lib/cimgui_colortextedit/%.cpp
	mkdir -p ${@D}
	${CXX} ${VENDOR_CXXFLAGS} ${INCLUDE} $(IMGUI_INC) $(IMTEXT_INC) $(IMGUI_DEFS) -c $< -o $@

# ======================== TESTS =========================
${OBJ_DIR}/%.test.o: %.c
	mkdir -p ${@D}
	${GCC} ${CFLAGS} ${INCLUDE} $(CRIT_INC) -I ${TEST_DIR} ${COV_FLAGS} -c $< -o $@

${OBJ_DIR}/tests/%.o: tests/%.c
	mkdir -p ${@D}
	${GCC} ${CFLAGS} ${INCLUDE} $(CRIT_INC) -I ${TEST_DIR} ${COV_FLAGS} -c $< -o $@

${TEST_NAME}: ${TEST_OBJS} ${LIBFT_LIB}
	${GCC} ${CFLAGS} -no-pie ${COV_FLAGS} ${TEST_OBJS} ${LIBFT_LIB} $(CRIT_LIBS) -Wl,-rpath,$(CRIT_LIBDIR) -o ${TEST_NAME}

test: ${TEST_NAME}

test-run: test
	./${TEST_NAME} --verbose --color=always

# ======================== COVERAGE =======================
coverage: test-run
	@echo "Generating coverage reports..."
	@$(GCOVR) -r . --gcov-executable $(GCOV) --exclude 'lib/' --exclude 'tests/' --json-summary coverage.json
	@$(GCOVR) -r . --gcov-executable $(GCOV) --exclude 'lib/' --exclude 'tests/' --txt --txt-metric branch
	@echo "✅ Coverage reports generated"

# ======================== LIBS ===========================
$(LIBFT_LIB):
	make -s -C ${LIBFT_DIR} --no-print-directory

$(MLX42_LIB):
	cmake -S $(MLX42_DIR) -B $(MLX42_BUILD)
	cmake --build $(MLX42_BUILD) -j4

# ======================== CLEAN ==========================
clean:
	${RM} ${OBJ_DIR}
	${RM} ${TEST_NAME}
	${RM} coverage.json
	${RM} ${XPM42_FILES}
	${RM} ${EDITOR_NAME}
	make -s -C ${LIBFT_DIR} clean --no-print-directory

fclean: clean
	${RM} ${BUILD_DIR}
	${RM} ${NAME}
	${RM} ${EDITOR_NAME}
	make -s -C ${LIBFT_DIR} fclean --no-print-directory
	${RM} ${MLX42_LIB}

re: fclean all

.PHONY: all editor clean fclean re test test-run coverage
