# ImGuiColorTextEdit C Interface
C interface for [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit), a syntax highlighting text editor for ImGui, on top of [cimgui](https://github.com/cimgui/cimgui).

![image](https://github.com/alfredbaudisch/cimgui_colortextedit/assets/248383/7d83bbc5-527a-443b-ae2f-a9ff66c33b71)

Alternative project, auto-generated bindings: https://github.com/cimgui/cimCTE

## Usage

```c
#include "cimcolortextedit.h"

TextEditor *editor = ImColorTextEdit_TextEditor();
ImColorTextEdit_TextEditor_SetText(editor, "Some text\r\nSome other text\r\n{\"some_json\": true}\r\n// Code comment");

// In the render loop...
ImColorTextEdit_TextEditor_Render(editor, "TextEditor");
```

## Building
```
make static
```

or

```
cmake -S . -B build
cmake --build build --config Release
```

### Example linking to project
After building the library as per instructions above, link to project with:

```
CIMGUI_PATH = C:/Dev/middleware/clibs/cimgui
CIMGUI_TEXT_EDIT_PATH = C:/Dev/middleware/clibs/cimcolortextedit

#...
INCLUDE_PATHS += -I$(CIMGUI_PATH) -I$(CIMGUI_TEXT_EDIT_PATH)

#...
LDFLAGS = -L. -L$(CIMGUI_TEXT_EDIT_PATH)

#...
LDLIBS = -lcimgui -lstdc++
```
