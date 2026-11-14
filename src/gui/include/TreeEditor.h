#pragma once
#include "raylib.h"
#include <string>

// Side panel with a multi-line text box and a "Generate" button,
// drawn with plain raylib calls.
class TreeEditor {
public:
    explicit TreeEditor(std::string initialText);

    // While focused, the editor receives keyboard and mouse input.
    // When unfocused it is still drawn (dimmed) but ignores input.
    bool focused = true;

    // Handles mouse and keyboard input. Call once per frame while focused.
    // Returns true when the user clicks "Generate" or presses Ctrl/Cmd+Enter.
    bool Update();
    void Draw() const;

    // True if the point lies on the panel (used to tell panel clicks from 3D-view clicks).
    bool Contains(Vector2 p) const { return CheckCollisionPointRec(p, Panel()); }

    // Loads the UI font (call after InitWindow) and frees it (call before CloseWindow).
    // Falls back to raylib's built-in font if no TTF is found.
    void LoadFont();
    void UnloadFont();

    const std::string& GetText() const { return text; }
    void SetStatus(std::string message, bool isError);

private:
    std::string text;
    size_t cursor;          // byte index into text
    std::string status;
    bool statusIsError = false;
    Font font = { 0 };
    bool ownsFont = false;

    float Measure(const std::string& s, float size) const;
    void  Text(const std::string& s, float x, float y, float size, Color color) const;

    Rectangle Panel() const;
    Rectangle TextBox() const;
    Rectangle Button() const;

    void Insert(const std::string& s);
    size_t LineStart(size_t i) const;
    size_t LineEnd(size_t i) const;
    void MoveVertical(int direction);
    size_t IndexAtPoint(Vector2 p) const;
};
