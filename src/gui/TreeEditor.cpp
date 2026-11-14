#include "include/TreeEditor.h"

#include <algorithm>
#include <utility>

namespace {
constexpr float kFontSize   = 18.0f;    // editor text
constexpr float kLineHeight = 22.0f;
constexpr float kTitleSize  = 22.0f;
constexpr float kStatusSize = 15.0f;
constexpr float kHelpSize   = 13.0f;
constexpr float kHelpLineH  = 17.0f;
constexpr float kPad        = 12.0f;
constexpr float kPanelWidth = 460.0f;

// Glyphs are rasterized at this size and scaled down when drawn,
// so text stays sharp on HiDPI (2x) screens.
constexpr int kFontLoadSize = 48;

// First file that exists wins. Drop any .ttf at res/fonts/editor.ttf to override.
const char* kFontCandidates[] = {
    "res/fonts/editor.ttf",
    "/System/Library/Fonts/Supplemental/Andale Mono.ttf",          // macOS
    "/System/Library/Fonts/Supplemental/Courier New.ttf",          // macOS
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",         // Debian/Ubuntu
    "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf",  // Fedora
};

const char* kHelp[] = {
    "sphere(cx cy cz, potential)",
    "capsule(ax ay az, bx by bz, potential)",
    "union | blend | difference | intersection(a, b, ...)",
    "# comment    Ctrl/Cmd + Enter: generate, S: save, O: load",
    "Click the 3D view: move camera   TAB: back here",
    "Drop a file on the window to load it",
};
constexpr int kHelpLines = sizeof(kHelp) / sizeof(kHelp[0]);

bool CtrlDown() {
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
           IsKeyDown(KEY_LEFT_SUPER)   || IsKeyDown(KEY_RIGHT_SUPER);
}

bool Pressed(int key) {
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}

bool Clicked(Rectangle r) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
}

// Clipboard text reduced to what the editor can display.
std::string ClipboardText(bool allowNewlines) {
    std::string s;
    if (const char* clip = GetClipboardText())
        for (const char* p = clip; *p; p++)
            if ((allowNewlines && *p == '\n') || (*p >= 32 && *p < 127)) s += *p;
    return s;
}

constexpr float kButtonH = 36.0f;
constexpr float kRowGap  = 8.0f;
constexpr float kPromptH = 32.0f;
constexpr float kStatusH = 22.0f;
} // namespace

TreeEditor::TreeEditor(std::string initialText)
    : text(std::move(initialText)), cursor(text.size()) {}

void TreeEditor::LoadFont() {
    font = GetFontDefault();
    for (const char* path : kFontCandidates) {
        if (!FileExists(path)) continue;
        Font f = LoadFontEx(path, kFontLoadSize, nullptr, 0);
        if (f.texture.id != 0 && f.texture.id != GetFontDefault().texture.id) {
            SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
            font = f;
            ownsFont = true;
            return;
        }
    }
}

void TreeEditor::UnloadFont() {
    if (ownsFont) ::UnloadFont(font);
    font = GetFontDefault();
    ownsFont = false;
}

float TreeEditor::Measure(const std::string& s, float size) const {
    return MeasureTextEx(font, s.c_str(), size, 0).x;
}

void TreeEditor::Text(const std::string& s, float x, float y, float size, Color color) const {
    DrawTextEx(font, s.c_str(), { x, y }, size, 0, color);
}

void TreeEditor::SetStatus(std::string message, bool isError) {
    status = std::move(message);
    statusIsError = isError;
}

void TreeEditor::SetText(std::string newText) {
    text = std::move(newText);
    cursor = text.size();
}

// ---------------------------------------------------------------------------
// Layout, from the bottom of the panel up: help text, status line,
// file prompt row, button row. The text box fills the rest.
// ---------------------------------------------------------------------------

Rectangle TreeEditor::Panel() const {
    return { kPad, kPad, kPanelWidth, (float)GetScreenHeight() - 2 * kPad };
}

Rectangle TreeEditor::GenerateButton() const {
    Rectangle p = Panel();
    float helpHeight = kHelpLines * kHelpLineH;
    float y = p.y + p.height - kPad - helpHeight - kStatusH - kRowGap - kPromptH - kRowGap - kButtonH;
    return { p.x + kPad, y, 130, kButtonH };
}

Rectangle TreeEditor::LoadButton() const {
    Rectangle g = GenerateButton();
    return { g.x + g.width + kRowGap, g.y, 100, kButtonH };
}

Rectangle TreeEditor::SaveButton() const {
    Rectangle l = LoadButton();
    return { l.x + l.width + kRowGap, l.y, 120, kButtonH };
}

Rectangle TreeEditor::PromptRow() const {
    Rectangle p = Panel();
    Rectangle g = GenerateButton();
    return { p.x + kPad, g.y + g.height + kRowGap, p.width - 2 * kPad, kPromptH };
}

Rectangle TreeEditor::PromptCancel() const {
    Rectangle r = PromptRow();
    return { r.x + r.width - 70, r.y, 70, r.height };
}

Rectangle TreeEditor::PromptOk() const {
    Rectangle c = PromptCancel();
    return { c.x - kRowGap - 50, c.y, 50, c.height };
}

const char* TreeEditor::PromptLabel() const {
    return prompt == PromptMode::Load ? "Load:" : "Save as:";
}

Rectangle TreeEditor::PromptInput() const {
    Rectangle r = PromptRow();
    Rectangle ok = PromptOk();
    float labelW = Measure("Save as:", kStatusSize) + kRowGap;
    return { r.x + labelW, r.y, ok.x - kRowGap - (r.x + labelW), r.height };
}

Rectangle TreeEditor::TextBox() const {
    Rectangle p = Panel();
    Rectangle b = GenerateButton();
    float top = p.y + kTitleSize + 2 * kPad;
    return { p.x + kPad, top, p.width - 2 * kPad, b.y - kPad - top };
}

// ---------------------------------------------------------------------------
// Text editing helpers
// ---------------------------------------------------------------------------

void TreeEditor::Insert(const std::string& s) {
    text.insert(cursor, s);
    cursor += s.size();
}

size_t TreeEditor::LineStart(size_t i) const {
    while (i > 0 && text[i - 1] != '\n') i--;
    return i;
}

size_t TreeEditor::LineEnd(size_t i) const {
    while (i < text.size() && text[i] != '\n') i++;
    return i;
}

// Moves the cursor one line up (-1) or down (+1), keeping the same column
// when the target line is long enough.
void TreeEditor::MoveVertical(int direction) {
    size_t start = LineStart(cursor);
    size_t column = cursor - start;

    size_t target;
    if (direction < 0) {
        if (start == 0) { cursor = 0; return; }
        target = LineStart(start - 1);
    } else {
        size_t end = LineEnd(cursor);
        if (end == text.size()) { cursor = end; return; }
        target = end + 1;
    }
    cursor = std::min(target + column, LineEnd(target));
}

// Text index closest to a point inside the text box.
size_t TreeEditor::IndexAtPoint(Vector2 p) const {
    Rectangle box = TextBox();
    int line = std::max(0, (int)((p.y - box.y - 6) / kLineHeight));

    size_t start = 0;
    for (int l = 0; l < line; l++) {
        size_t end = LineEnd(start);
        if (end == text.size()) break;   // clicked below the last line
        start = end + 1;
    }

    size_t end = LineEnd(start);
    float x = p.x - box.x - 8;
    std::string prefix;
    for (size_t i = start; i < end; i++) {
        float before = Measure(prefix, kFontSize);
        prefix += text[i];
        float after = Measure(prefix, kFontSize);
        if (x < (before + after) / 2) return i;
    }
    return end;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void TreeEditor::OpenPrompt(PromptMode mode) {
    prompt = mode;
    promptText = filePath;
}

// While the file prompt is open it takes all keyboard input:
// Enter / OK confirms, ESC / Cancel closes it.
EditorAction TreeEditor::UpdatePrompt() {
    EditorAction confirmed = prompt == PromptMode::Load ? EditorAction::Load : EditorAction::Save;

    for (int c = GetCharPressed(); c != 0; c = GetCharPressed())
        if (c >= 32 && c < 127) promptText += (char)c;

    if (CtrlDown() && IsKeyPressed(KEY_V)) promptText += ClipboardText(false);
    if (Pressed(KEY_BACKSPACE) && !promptText.empty()) promptText.pop_back();

    bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || Clicked(PromptOk());
    bool cancel  = IsKeyPressed(KEY_ESCAPE) || Clicked(PromptCancel());

    if (cancel) {
        prompt = PromptMode::None;
        return EditorAction::None;
    }
    if (confirm && !promptText.empty()) {
        prompt = PromptMode::None;
        actionPath = promptText;
        return confirmed;
    }
    return EditorAction::None;
}

EditorAction TreeEditor::Update() {
    // The buttons work in both states; clicking Load/Save again switches the prompt
    if (Clicked(GenerateButton())) { prompt = PromptMode::None; return EditorAction::Generate; }
    if (Clicked(LoadButton()))     { OpenPrompt(PromptMode::Load); return EditorAction::None; }
    if (Clicked(SaveButton()))     { OpenPrompt(PromptMode::Save); return EditorAction::None; }

    if (PromptOpen()) return UpdatePrompt();

    EditorAction action = EditorAction::None;
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, TextBox()))
        cursor = IndexAtPoint(mouse);

    // Typed characters (printable ASCII only).
    for (int c = GetCharPressed(); c != 0; c = GetCharPressed()) {
        if (c >= 32 && c < 127) Insert(std::string(1, (char)c));
    }

    if (Pressed(KEY_ENTER) || Pressed(KEY_KP_ENTER)) {
        if (CtrlDown()) action = EditorAction::Generate;
        else Insert("\n");
    }

    if (CtrlDown() && IsKeyPressed(KEY_V)) Insert(ClipboardText(true));
    if (CtrlDown() && IsKeyPressed(KEY_S)) OpenPrompt(PromptMode::Save);
    if (CtrlDown() && IsKeyPressed(KEY_O)) OpenPrompt(PromptMode::Load);

    if (Pressed(KEY_BACKSPACE) && cursor > 0) text.erase(--cursor, 1);
    if (Pressed(KEY_DELETE) && cursor < text.size()) text.erase(cursor, 1);
    if (Pressed(KEY_LEFT) && cursor > 0) cursor--;
    if (Pressed(KEY_RIGHT) && cursor < text.size()) cursor++;
    if (Pressed(KEY_UP)) MoveVertical(-1);
    if (Pressed(KEY_DOWN)) MoveVertical(+1);
    if (IsKeyPressed(KEY_HOME)) cursor = LineStart(cursor);
    if (IsKeyPressed(KEY_END)) cursor = LineEnd(cursor);

    return action;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void TreeEditor::DrawButton(Rectangle r, const char* label, bool enabled) const {
    bool hover = enabled && focused && CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, hover ? SKYBLUE : LIGHTGRAY);
    DrawRectangleLinesEx(r, 1, DARKGRAY);
    float size = 18;
    float tw = Measure(label, size);
    Text(label, r.x + (r.width - tw) / 2, r.y + (r.height - size) / 2, size, DARKBLUE);
}

void TreeEditor::Draw() const {
    Rectangle panel = Panel();
    DrawRectangleRec(panel, Fade(RAYWHITE, focused ? 0.92f : 0.6f));
    DrawRectangleLinesEx(panel, focused ? 2 : 1, focused ? DARKBLUE : GRAY);
    Text("Blob tree", panel.x + kPad, panel.y + kPad, kTitleSize, DARKGRAY);

    bool blinkOn = (int)(GetTime() * 2) % 2 == 0;

    // Text box
    Rectangle box = TextBox();
    DrawRectangleRec(box, WHITE);
    DrawRectangleLinesEx(box, 1, DARKGRAY);

    BeginScissorMode((int)box.x, (int)box.y, (int)box.width, (int)box.height);
    {
        float x = box.x + 8;
        float y = box.y + 6;
        size_t start = 0;
        while (true) {
            size_t end = LineEnd(start);
            std::string line = text.substr(start, end - start);
            bool comment = line.find_first_not_of(" \t") != std::string::npos &&
                           line[line.find_first_not_of(" \t")] == '#';
            Text(line, x, y, kFontSize, comment ? GRAY : BLACK);

            // Blinking cursor (hidden while the file prompt has the keyboard)
            if (focused && !PromptOpen() && cursor >= start && cursor <= end && blinkOn) {
                std::string before = text.substr(start, cursor - start);
                float cx = x + Measure(before, kFontSize);
                DrawRectangleRec({ cx, y - 1, 2, kFontSize + 2 }, MAROON);
            }

            if (end == text.size()) break;
            start = end + 1;
            y += kLineHeight;
        }
    }
    EndScissorMode();

    // Buttons
    DrawButton(GenerateButton(), "Generate", true);
    DrawButton(LoadButton(), "Load...", true);
    DrawButton(SaveButton(), "Save as...", true);

    // File name prompt
    if (PromptOpen()) {
        Rectangle row = PromptRow();
        Rectangle in = PromptInput();
        Text(PromptLabel(), row.x, row.y + (row.height - kStatusSize) / 2, kStatusSize, DARKGRAY);

        DrawRectangleRec(in, WHITE);
        DrawRectangleLinesEx(in, 1, DARKBLUE);
        BeginScissorMode((int)in.x, (int)in.y, (int)in.width, (int)in.height);
        {
            // Keep the end of a long path visible
            float tw = Measure(promptText, kStatusSize);
            float tx = std::min(in.x + 6, in.x + in.width - 8 - tw);
            float ty = in.y + (in.height - kStatusSize) / 2;
            Text(promptText, tx, ty, kStatusSize, BLACK);
            if (focused && blinkOn)
                DrawRectangleRec({ tx + tw + 1, ty - 1, 2, kStatusSize + 2 }, MAROON);
        }
        EndScissorMode();

        DrawButton(PromptOk(), "OK", true);
        DrawButton(PromptCancel(), "Cancel", true);
    }

    // Status line
    if (!status.empty()) {
        Rectangle row = PromptRow();
        Text(status, panel.x + kPad, row.y + row.height + kRowGap, kStatusSize,
             statusIsError ? RED : DARKGREEN);
    }

    // Syntax help
    float hy = panel.y + panel.height - kPad - kHelpLines * kHelpLineH;
    for (int i = 0; i < kHelpLines; i++)
        Text(kHelp[i], panel.x + kPad, hy + i * kHelpLineH, kHelpSize, GRAY);
}
