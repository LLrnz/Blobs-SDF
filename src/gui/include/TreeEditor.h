#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// What the user asked for this frame (returned by TreeEditor::Update).
enum class EditorAction { None, Generate, Load, Save };

// Side panel with a multi-line text box, a column of "insert" buttons
// (primitives and operators), "Generate" / "Load..." / "Save as..." buttons
// and a one-line file name prompt, drawn with plain raylib calls.
class TreeEditor {
public:
    explicit TreeEditor(std::string initialText);

    // While focused, the editor receives keyboard and mouse input.
    // When unfocused it is still drawn (dimmed) but ignores input.
    bool focused = true;

    // "Accel" checkbox: sort the tree and polygonize with the BVH. Toggling it regenerates.
    bool accelerate = true;

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
    enum class PromptMode { None, Load, Save, Random };

    std::string text;
    // The selection is [min(anchor, cursor), max(anchor, cursor)); empty when equal.
    size_t cursor;
    size_t anchor;
    bool dragging = false;          // mouse selection in progress
    double lastClickTime = -1.0;    // for double-click detection
    size_t lastClickIndex = 0;
    float scrollY = 0.0f;           // vertical scroll of the text box, in pixels

    std::string status;
    bool statusIsError = false;
    Font font = { 0 };
    bool ownsFont = false;

    PromptMode prompt = PromptMode::None;
    std::string promptText;
    std::string filePath = "my_tree.txt";
    std::string actionPath;
    int randomCount = 10;           // last blob count used by "Random..."

    // Drawing helpers
    float Measure(const std::string& s, float size) const;
    void  Text(const std::string& s, float x, float y, float size, Color color) const;
    void  DrawButton(Rectangle r, const char* label, bool enabled) const;
    void  DrawCheckbox(Rectangle r, const char* label, bool checked) const;

    // Layout
    Rectangle Panel() const;
    Rectangle TextBox() const;
    Rectangle InsertButton(int i) const;
    Rectangle GenerateButton() const;
    Rectangle LoadButton() const;
    Rectangle SaveButton() const;
    Rectangle RandomButton() const;
    Rectangle AccelerateCheckbox() const;   // box + label, the whole area is clickable
    Rectangle PromptRow() const;
    Rectangle PromptInput() const;
    Rectangle PromptOk() const;
    Rectangle PromptCancel() const;
    const char* PromptLabel() const;

    void OpenPrompt(PromptMode mode);
    static std::string RandomTree(int count);   // blend of `count` random primitives
    EditorAction UpdatePrompt();
    void UpdateTextBox();

    // Selection
    bool   HasSelection() const { return anchor != cursor; }
    size_t SelStart() const { return cursor < anchor ? cursor : anchor; }
    size_t SelEnd() const   { return cursor < anchor ? anchor : cursor; }
    void   MoveCursor(size_t pos, bool extendSelection);
    void   DeleteSelection();
    void   SelectWordAt(size_t i);

    // Undo / redo: full snapshots of the text and selection, taken before each edit.
    // Consecutive edits of the same kind (typing, or deleting with Backspace/Delete)
    // share one snapshot, so a typed word is undone in one step.
    struct Snapshot { std::string text; size_t cursor, anchor; };
    enum class EditKind { None, Typing, Deleting, Other };
    std::vector<Snapshot> undoStack, redoStack;
    EditKind lastEdit = EditKind::None;
    void Checkpoint(EditKind kind);     // call before modifying text
    void BreakUndoGroup() { lastEdit = EditKind::None; }
    void Undo();
    void Redo();

    // Editing
    void Insert(const std::string& s);                 // replaces the selection
    void InsertSnippet(const std::string& snippet);    // re-indented to the cursor's indentation
    std::string IndentBefore(size_t i) const;          // leading whitespace of i's line, up to i
    void IndentLines();
    void UnindentLines();

    // Lines and columns (tabs are expanded to the next multiple of kTabWidth)
    size_t LineStart(size_t i) const;
    size_t LineEnd(size_t i) const;
    int    LineIndex(size_t i) const;
    static std::string ExpandTabs(const std::string& s);
    float  XOf(size_t lineStart, size_t i) const;      // x offset of index i in its line
    size_t IndexAtX(size_t lineStart, float x) const;
    void   MoveVertical(int direction, bool extendSelection);
    size_t IndexAtPoint(Vector2 p) const;
    void   ScrollToCursor();
};
