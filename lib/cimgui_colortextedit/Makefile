OBJS = ../cimgui/cimgui.o
OBJS += ../cimgui/imgui/imgui.o
OBJS += ../cimgui/imgui/imgui_draw.o
OBJS += ../cimgui/imgui/imgui_demo.o
OBJS += ../cimgui/imgui/imgui_widgets.o
OBJS += ../cimgui/imgui/imgui_tables.o
OBJS += ImGuiColorTextEdit/TextEditor.o
OBJS += cimcolortextedit.o

CXXFLAGS=-O2 -fno-exceptions -fno-rtti -std=c++11 -DIMGUI_DISABLE_OBSOLETE_KEYIO=1

UNAME_S := $(shell uname -s)

AR := ar -rc

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"

	OUTPUTNAME = cimgui.so
	CXXFLAGS += -I./imgui/
	CXXFLAGS += -Wall
	CXXFLAGS += -shared -fPIC
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "macOS"

	OUTPUTNAME = cimgui.dylib
	CXXFLAGS += -I/usr/local/include
	CXXFLAGS += -Wall
	LINKFLAGS = -dynamiclib
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "Windows"

	OUTPUTNAME = cimgui.dll
	CXXFLAGS += -IC:/Dev/middleware/clibs/cimgui/imgui/ -IC:/Dev/middleware/clibs/cimgui/ -I./ImGuiColorTextEdit/
	CXXFLAGS += -Wall
	CXXFLAGS += -shared
	LINKFLAGS = -limm32
	CFLAGS = $(CXXFLAGS)
endif

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all:$(OUTPUTNAME)
	@echo Build complete for $(ECHO_MESSAGE)

static:$(OBJS)
	$(AR) libcimgui.a $(OBJS)

$(OUTPUTNAME):$(OBJS)
	$(CXX) -o $(OUTPUTNAME) $(OBJS) $(CXXFLAGS) $(LINKFLAGS)

clean:
ifeq ($(OS),Windows_NT)
	del /q $(subst /,\,$(OBJS))
else
	rm -f $(OBJS)
endif

fclean: clean
ifeq ($(OS),Windows_NT)
	del /q $(subst /,\,$(OUTPUTNAME))
else
	rm -f $(OUTPUTNAME)
endif

re: fclean all

.PHONY: all clean fclean re static
