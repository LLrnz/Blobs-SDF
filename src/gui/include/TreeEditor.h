#pragma once
#include "raylib.h"
#include <string>

// What the user asked for this frame (returned by TreeEditor::Update).
enum class EditorAction { None, Generate, Load, Save };

// Side panel with a multi-line text box, "Generate" / "Load..." / "Save as..."
// buttons and a one-line file name prompt, drawn with plain raylib calls.
class TreeEditor {
public:
    explicit TreeEditor(std::string initialText);

    // While focused, the editor receives keyboard and mouse input.
    // When unfocused it is still drawn (dimmed) but ignores input.
    bool focused = true;

    // Handles mouse and keyboard input. Call once per frame while focused.
    // For Load / Save, the chosen file is available from ActionPath().
    EditorAction Update();
    void Draw() const;

    // Path typed in the file prompt for the last Load / Save action.
    const std::string& ActionPath() const { return actionPath; }

    // True while the file name prompt is open (it uses ESC to cancel).
    bool PromptOpen() const { return prompt != PromptMode::None; }

    // True if the point lies on the panel (used to tell panel clicks from 3D-view clicks).
    bool Contains(Vector2 p) const { return CheckCollisionPointRec(p, Panel()); }

    // Loads the UI font (call after InitWindow) and frees it (call before CloseWindow).
    // Falls back to raylib's built-in font if no TTF is found.
    void LoadFont();
    void UnloadFont();

    const std::string& GetText() const { return text; }
    void SetText(std::string newText);
    void SetStatus(std::string message, bool isError);

    // File used to prefill the prompt next time (last file loaded or saved).
    void SetFilePath(std::string path) { filePath = std::move(path); }

private:
    enum class PromptMode { None, Load, Save };

    std::string text;
    size_t cursor;          // byte index into text
    std::string status;
    bool statusIsError = false;
    Font font = { 0 };
    bool ownsFont = false;

    PromptMode prompt = PromptMode::None;
    std::string promptText;
    std::string filePath = "my_tree.txt";
    std::string actionPath;

    float Measure(const std::string& s, float size) const;
    void  Text(const std::string& s, float x, float y, float size, Color color) const;
    void  DrawButton(Rectangle r, const char* label, bool enabled) const;

    Rectangle Panel() const;
    Rectangle TextBox() const;
    Rectangle GenerateButton() const;
    Rectangle LoadButton() const;
    Rectangle SaveButton() const;
    Rectangle PromptRow() const;
    Rectangle PromptInput() const;
    Rectangle PromptOk() const;
    Rectangle PromptCancel() const;
    const char* PromptLabel() const;

    void OpenPrompt(PromptMode mode);
    EditorAction UpdatePrompt();

    void Insert(const std::string& s);
    size_t LineStart(size_t i) const;
    size_t LineEnd(size_t i) const;
    void MoveVertical(int direction);
    size_t IndexAtPoint(Vector2 p) const;
};
