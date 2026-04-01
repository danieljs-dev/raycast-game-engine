#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "cimcolortextedit.h"

CIMGUI_API TextEditor* ImColorTextEdit_TextEditor(void)
{
    return IM_NEW(TextEditor)();
}

CIMGUI_API void ImColorTextEdit_destroy(TextEditor* self)
{
    IM_DELETE(self);
}

CIMGUI_API void ImColorTextEdit_TextEditor_Render(TextEditor* self, const char* aTitle)
{
    self->Render(aTitle);
}

CIMGUI_API void ImColorTextEdit_TextEditor_RenderSize(TextEditor* self, const char* aTitle, const ImVec2* aSize)
{
    self->Render(aTitle, *aSize);
}

CIMGUI_API void ImColorTextEdit_TextEditor_RenderSizeBorder(TextEditor* self, const char* aTitle, const ImVec2* aSize, bool aBorder)
{
    self->Render(aTitle, *aSize, aBorder);
}

CIMGUI_API void ImColorTextEdit_TextEditor_SetText(TextEditor* self, const char *text)
{
    self->SetText(text);
}

CIMGUI_API const char* ImColorTextEdit_TextEditor_GetText(TextEditor* self)
{
    /*
     * IMPORTANT: TextEditor::GetText() returns a std::string by value.
     * Returning c_str() from that temporary would dangle and corrupt callers.
     * Keep a stable buffer per-thread.
     */
    static thread_local std::string	stable;

    if (!self)
        return "";
    stable = self->GetText();
    return stable.c_str();
}
