#include "include/TreeEditor.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

namespace {
constexpr float kFontSize   = 18.0f;    // editor text
constexpr float kLineHeight = 22.0f;
constexpr float kTitleSize  = 22.0f;
constexpr float kStatusSize = 15.0f;
constexpr float kHelpSize   = 13.0f;
constexpr float kHelpLineH  = 17.0f;
constexpr float kPad        = 12.0f;
constexpr float kPanelWidth = 600.0f;
constexpr float kTextInsetX = 8.0f;     // text offset inside the text box
constexpr float kTextInsetY = 6.0f;
constexpr int   kTabWidth   = 4;        // tab stops every 4 columns

constexpr float kButtonH  = 36.0f;
constexpr float kRowGap   = 8.0f;
constexpr float kPromptH  = 32.0f;
constexpr float kStatusH  = 22.0f;
constexpr float kColumnW  = 130.0f;     // width of the insert-button column
constexpr float kInsertH  = 28.0f;     // all insert buttons must fit beside the text box
constexpr float kInsertGap = 4.0f;

constexpr double kDoubleClickTime = 0.35;
constexpr size_t kMaxUndo = 500;
constexpr int    kMaxRandomBlobs = 1000;  // upper limit for the "Random..." button        // undo steps kept

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
    "sphere(cx cy cz, potential [, r g b])",
    "capsule(ax ay az, bx by bz, potential [, r g b])   colors: 0-255",
    "union | blend | difference | intersection(a, b, ...)",
    "translate(x y z, a)  rotateX|Y|Z(deg, a)  scale(x y z, a)",
    "twist(deg/unit y, a)  taper(k, a)  bend(deg/unit x, a)",
    "revolution(a): revolves a's z = 0 profile around y",
    "# comment    Ctrl/Cmd + Enter: generate, S: save, O: load",
    "Tab / Shift+Tab: indent    Double-click: select word",
    "Ctrl/Cmd + Z: undo, Shift+Z or Y: redo",
    "Click 3D view / ESC: camera (WASD, Space/Shift: up/down)",
    "Drop a file to load it  F1: panel  F2: screenshot  F3: bbox",
};
constexpr int kHelpLines = sizeof(kHelp) / sizeof(kHelp[0]);

// Snippets for the insert buttons. '\n' lines are re-indented to the cursor's
// indentation; operators get their first NODE placeholder selected.
struct InsertItem {
    const char* label;
    const char* snippet;
    bool gapBefore;     // visual separator between primitives and operators
};
const InsertItem kInsertItems[] = {
    { "sphere",       "sphere(0 0 0, 1, COLOR)",                   false },
    { "capsule",      "capsule(0 0 0, 0 1 0, 1, COLOR)",           false },
    { "blend",        "blend(\n\tNODE,\n\tNODE\n)",                true  },
    { "union",        "union(\n\tNODE,\n\tNODE\n)",                false },
    { "difference",   "difference(\n\tNODE,\n\tNODE\n)",           false },
    { "intersection", "intersection(\n\tNODE,\n\tNODE\n)",         false },
    { "translate",    "translate(0 0 0,\n\tNODE\n)",               true  },
    { "rotateX",      "rotateX(45,\n\tNODE\n)",                    false },
    { "rotateY",      "rotateY(45,\n\tNODE\n)",                    false },
    { "rotateZ",      "rotateZ(45,\n\tNODE\n)",                    false },
    { "scale",        "scale(1 1 1,\n\tNODE\n)",                   false },
    { "twist",        "twist(45,\n\tNODE\n)",                      false },
    { "taper",        "taper(0.5,\n\tNODE\n)",                     false },
    { "bend",         "bend(30,\n\tNODE\n)",                       false },
    { "revolution",   "revolution(\n\tNODE\n)",                    false },
};
constexpr int kInsertCount = sizeof(kInsertItems) / sizeof(kInsertItems[0]);

const char* kPlaceholder = "NODE";
const char* kColorToken = "COLOR";     // replaced by a random "r g b" when inserted

// A random, fairly saturated and bright color as "r g b", so primitives are easy to tell apart.
std::string RandomColor() {
    Color c = ColorFromHSV((float)GetRandomValue(0, 359), 0.65f, 0.9f);
    return std::to_string(c.r) + " " + std::to_string(c.g) + " " + std::to_string(c.b);
}

bool CtrlDown() {
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
           IsKeyDown(KEY_LEFT_SUPER)   || IsKeyDown(KEY_RIGHT_SUPER);
}

bool ShiftDown() {
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

bool Pressed(int key) {
    return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}

bool Clicked(Rectangle r) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
}

// Clipboard text reduced to what the editor can display.
std::string ClipboardText(bool multiline) {
    std::string s;
    if (const char* clip = GetClipboardText())
        for (const char* p = clip; *p; p++)
            if ((multiline && (*p == '\n' || *p == '\t')) || (*p >= 32 && *p < 127)) s += *p;
    return s;
}

bool IsWordChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-';
}
} // namespace

TreeEditor::TreeEditor(std::string initialText)
    : text(std::move(initialText)), cursor(text.size()), anchor(text.size()) {}

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
    Checkpoint(EditKind::Other);    // loading a file can be undone
    text = std::move(newText);
    cursor = anchor = text.size();
    scrollY = 0;
}

// ---------------------------------------------------------------------------
// Layout, from the bottom of the panel up: help text, status line,
// file prompt row, button row. The text box fills the rest, with the
// insert-button column on its right.
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

Rectangle TreeEditor::RandomButton() const {
    Rectangle s = SaveButton();
    return { s.x + s.width + kRowGap, s.y, 120, kButtonH };
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
    switch (prompt) {
        case PromptMode::Load:   return "Load:";
        case PromptMode::Random: return "Blobs:";
        default:                 return "Save as:";
    }
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
    return { p.x + kPad, top, p.width - 3 * kPad - kColumnW, b.y - kPad - top };
}

Rectangle TreeEditor::InsertButton(int i) const {
    Rectangle box = TextBox();
    float y = box.y;
    for (int k = 0; k <= i; k++) {
        if (kInsertItems[k].gapBefore) y += 2 * kInsertGap;
        if (k < i) y += kInsertH + kInsertGap;
    }
    return { box.x + box.width + kPad, y, kColumnW, kInsertH };
}

// ---------------------------------------------------------------------------
// Lines, columns and tabs
// ---------------------------------------------------------------------------

size_t TreeEditor::LineStart(size_t i) const {
    while (i > 0 && text[i - 1] != '\n') i--;
    return i;
}

size_t TreeEditor::LineEnd(size_t i) const {
    while (i < text.size() && text[i] != '\n') i++;
    return i;
}

int TreeEditor::LineIndex(size_t i) const {
    return (int)std::count(text.begin(), text.begin() + std::min(i, text.size()), '\n');
}

// Replaces tabs by spaces up to the next tab stop (s must be a single line).
std::string TreeEditor::ExpandTabs(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\t') out.append(kTabWidth - out.size() % kTabWidth, ' ');
        else out += c;
    }
    return out;
}

float TreeEditor::XOf(size_t lineStart, size_t i) const {
    return Measure(ExpandTabs(text.substr(lineStart, i - lineStart)), kFontSize);
}

// Index in the line starting at lineStart closest to horizontal offset x.
size_t TreeEditor::IndexAtX(size_t lineStart, float x) const {
    size_t end = LineEnd(lineStart);
    float before = 0;
    for (size_t i = lineStart; i < end; i++) {
        float after = XOf(lineStart, i + 1);
        if (x < (before + after) / 2) return i;
        before = after;
    }
    return end;
}

// Moves the cursor one line up (-1) or down (+1), keeping the same on-screen x.
void TreeEditor::MoveVertical(int direction, bool extendSelection) {
    size_t start = LineStart(cursor);
    float x = XOf(start, cursor);

    size_t target;
    if (direction < 0) {
        if (start == 0) { MoveCursor(0, extendSelection); return; }
        target = LineStart(start - 1);
    } else {
        size_t end = LineEnd(cursor);
        if (end == text.size()) { MoveCursor(end, extendSelection); return; }
        target = end + 1;
    }
    MoveCursor(IndexAtX(target, x), extendSelection);
}

// Text index closest to a screen point (points outside the box clamp to the nearest line).
size_t TreeEditor::IndexAtPoint(Vector2 p) const {
    Rectangle box = TextBox();
    int line = std::max(0, (int)((p.y - box.y - kTextInsetY + scrollY) / kLineHeight));

    size_t start = 0;
    for (int l = 0; l < line; l++) {
        size_t end = LineEnd(start);
        if (end == text.size()) break;   // below the last line
        start = end + 1;
    }
    return IndexAtX(start, p.x - box.x - kTextInsetX);
}

void TreeEditor::ScrollToCursor() {
    Rectangle box = TextBox();
    float visible = box.height - 2 * kTextInsetY;
    float top = LineIndex(cursor) * kLineHeight;
    if (top < scrollY) scrollY = top;
    if (top + kLineHeight > scrollY + visible) scrollY = top + kLineHeight - visible;
}

// ---------------------------------------------------------------------------
// Undo / redo
// ---------------------------------------------------------------------------

void TreeEditor::Checkpoint(EditKind kind) {
    bool continuesGroup = kind != EditKind::Other && kind == lastEdit;
    lastEdit = kind;
    if (continuesGroup) return;

    undoStack.push_back({ text, cursor, anchor });
    if (undoStack.size() > kMaxUndo) undoStack.erase(undoStack.begin());
    redoStack.clear();
}

void TreeEditor::Undo() {
    if (undoStack.empty()) return;
    redoStack.push_back({ text, cursor, anchor });
    Snapshot s = std::move(undoStack.back());
    undoStack.pop_back();
    text = std::move(s.text);
    cursor = s.cursor;
    anchor = s.anchor;
    BreakUndoGroup();
}

void TreeEditor::Redo() {
    if (redoStack.empty()) return;
    undoStack.push_back({ text, cursor, anchor });
    Snapshot s = std::move(redoStack.back());
    redoStack.pop_back();
    text = std::move(s.text);
    cursor = s.cursor;
    anchor = s.anchor;
    BreakUndoGroup();
}

// ---------------------------------------------------------------------------
// Selection and editing
// ---------------------------------------------------------------------------

void TreeEditor::MoveCursor(size_t pos, bool extendSelection) {
    cursor = std::min(pos, text.size());
    if (!extendSelection) anchor = cursor;
}

void TreeEditor::DeleteSelection() {
    if (!HasSelection()) return;
    size_t s = SelStart();
    text.erase(s, SelEnd() - s);
    cursor = anchor = s;
}

void TreeEditor::SelectWordAt(size_t i) {
    size_t a = i, b = i;
    while (a > 0 && IsWordChar(text[a - 1])) a--;
    while (b < text.size() && IsWordChar(text[b])) b++;
    anchor = a;
    cursor = b;
}

void TreeEditor::Insert(const std::string& s) {
    DeleteSelection();
    text.insert(cursor, s);
    cursor += s.size();
    anchor = cursor;
}

// Leading whitespace of the line containing i, but not past i.
std::string TreeEditor::IndentBefore(size_t i) const {
    size_t start = LineStart(i);
    size_t end = start;
    while (end < i && (text[end] == ' ' || text[end] == '\t')) end++;
    return text.substr(start, end - start);
}

void TreeEditor::InsertSnippet(const std::string& snippet) {
    std::string indent = IndentBefore(HasSelection() ? SelStart() : cursor);

    std::string s;
    for (char c : snippet) {
        s += c;
        if (c == '\n') s += indent;
    }

    DeleteSelection();
    size_t at = cursor;
    Insert(s);

    // Select the first placeholder of the inserted text, so the next button fills it
    size_t ph = text.find(kPlaceholder, at);
    if (ph != std::string::npos && ph < at + s.size()) {
        anchor = ph;
        cursor = ph + std::string(kPlaceholder).size();
    }
}

// Lines touched by the selection (or the cursor's line): [first line start, last line start].
static std::vector<size_t> SelectedLineStarts(const std::string& text, size_t selStart, size_t selEnd) {
    std::vector<size_t> starts;
    size_t s = selStart;
    while (s > 0 && text[s - 1] != '\n') s--;
    // A selection ending right at a line start doesn't include that line
    size_t last = (selEnd > selStart && selEnd > 0 && text[selEnd - 1] == '\n') ? selEnd - 1 : selEnd;
    while (true) {
        starts.push_back(s);
        size_t e = text.find('\n', s);
        if (e == std::string::npos || e >= last) break;
        s = e + 1;
    }
    return starts;
}

void TreeEditor::IndentLines() {
    std::vector<size_t> starts = SelectedLineStarts(text, SelStart(), SelEnd());
    size_t selStart = SelStart();
    // Insert from the last line up so earlier positions stay valid
    for (auto it = starts.rbegin(); it != starts.rend(); ++it) {
        size_t p = *it;
        text.insert(p, "\t");
        // Shift positions after the insertion; keep a selection that starts at
        // a line start covering the new tab
        if (cursor > p || (cursor == p && cursor != selStart)) cursor++;
        if (anchor > p || (anchor == p && anchor != selStart)) anchor++;
    }
}

void TreeEditor::UnindentLines() {
    std::vector<size_t> starts = SelectedLineStarts(text, SelStart(), SelEnd());
    for (auto it = starts.rbegin(); it != starts.rend(); ++it) {
        size_t p = *it;
        size_t n = 0;
        if (p < text.size() && text[p] == '\t') n = 1;
        else while (n < (size_t)kTabWidth && p + n < text.size() && text[p + n] == ' ') n++;
        if (n == 0) continue;
        text.erase(p, n);
        if (cursor > p) cursor -= std::min(n, cursor - p);
        if (anchor > p) anchor -= std::min(n, anchor - p);
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void TreeEditor::OpenPrompt(PromptMode mode) {
    prompt = mode;
    promptText = mode == PromptMode::Random ? std::to_string(randomCount) : filePath;
}

namespace {
float RandomFloat(float lo, float hi) {
    return lo + (hi - lo) * GetRandomValue(0, 1000000) / 1000000.0f;
}

std::string Fmt(float v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.2f", v);
    return buf;
}
}

// A blend of `count` random primitives (about 3/4 spheres, 1/4 short capsules), each
// with a random color. The scatter grows with the cube root of the count, so the
// density of blobs (and the size of the blended shape) stays reasonable for large counts.
std::string TreeEditor::RandomTree(int count) {
    float extent = 1.2f * std::cbrt((float)count);
    auto point = [&]() {
        return Fmt(RandomFloat(-extent, extent)) + " " +
               Fmt(RandomFloat(-extent, extent)) + " " +
               Fmt(RandomFloat(-extent, extent));
    };

    std::vector<std::string> blobs;
    for (int i = 0; i < count; i++) {
        std::string potential = Fmt(RandomFloat(0.5f, 1.2f));
        if (GetRandomValue(0, 3) == 0) {
            // Capsule: a second endpoint within one unit of the first
            float a[3], b[3];
            for (int k = 0; k < 3; k++) {
                a[k] = RandomFloat(-extent, extent);
                b[k] = a[k] + RandomFloat(-1.0f, 1.0f);
            }
            blobs.push_back("capsule(" + Fmt(a[0]) + " " + Fmt(a[1]) + " " + Fmt(a[2]) + ", " +
                            Fmt(b[0]) + " " + Fmt(b[1]) + " " + Fmt(b[2]) + ", " +
                            potential + ", " + RandomColor() + ")");
        }
        else {
            blobs.push_back("sphere(" + point() + ", " + potential + ", " + RandomColor() + ")");
        }
    }

    if (count == 1) return blobs[0] + "\n";   // blend needs at least two children
    std::string tree = "# " + std::to_string(count) + " random blobs\nblend(\n";
    for (int i = 0; i < count; i++)
        tree += "\t" + blobs[i] + (i + 1 < count ? ",\n" : "\n");
    return tree + ")\n";
}

// While the file prompt is open it takes all keyboard input:
// Enter / OK confirms, ESC / Cancel closes it.
EditorAction TreeEditor::UpdatePrompt() {
    EditorAction confirmed = prompt == PromptMode::Load ? EditorAction::Load : EditorAction::Save;
    bool random = prompt == PromptMode::Random;

    if (!CtrlDown())
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
    if (confirm && random) {
        // The text is replaced by the random tree (undoable), then generated
        char* end = nullptr;
        long n = std::strtol(promptText.c_str(), &end, 10);
        if (promptText.empty() || *end != '\0' || n < 1 || n > kMaxRandomBlobs) {
            SetStatus("Enter a number of blobs between 1 and " + std::to_string(kMaxRandomBlobs), true);
            return EditorAction::None;
        }
        prompt = PromptMode::None;
        randomCount = (int)n;
        SetText(RandomTree(randomCount));
        return EditorAction::Generate;
    }
    if (confirm && !promptText.empty()) {
        prompt = PromptMode::None;
        actionPath = promptText;
        return confirmed;
    }
    return EditorAction::None;
}

// Mouse and keyboard editing of the text box.
void TreeEditor::UpdateTextBox() {
    Rectangle box = TextBox();
    Vector2 mouse = GetMousePosition();
    bool shift = ShiftDown();
    bool ctrl = CtrlDown();
    bool edited = false;    // keyboard change: scroll so the cursor stays visible

    // Mouse wheel scrolling
    if (CheckCollisionPointRec(mouse, box)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            float contentH = (LineIndex(text.size()) + 1) * kLineHeight;
            float maxScroll = std::max(0.0f, contentH - (box.height - 2 * kTextInsetY));
            scrollY = std::clamp(scrollY - wheel * 3 * kLineHeight, 0.0f, maxScroll);
        }
    }

    // Click: place cursor (Shift extends), double-click: select word, drag: select
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, box)) {
        size_t i = IndexAtPoint(mouse);
        double now = GetTime();
        BreakUndoGroup();
        if (now - lastClickTime < kDoubleClickTime && i == lastClickIndex) {
            SelectWordAt(i);
            dragging = false;
            lastClickTime = -1.0;   // a third click starts over
        } else {
            MoveCursor(i, shift);
            dragging = true;
            lastClickTime = now;
            lastClickIndex = i;
        }
    }
    if (dragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) cursor = IndexAtPoint(mouse);
        else dragging = false;
    }

    // Typed characters (printable ASCII; Ctrl/Cmd shortcuts are handled below)
    if (!ctrl) {
        for (int c = GetCharPressed(); c != 0; c = GetCharPressed()) {
            if (c >= 32 && c < 127) {
                // Replacing a selection is its own step; plain typing groups together
                Checkpoint(HasSelection() ? EditKind::Other : EditKind::Typing);
                Insert(std::string(1, (char)c));
                edited = true;
            }
        }
    }

    // Enter keeps the current line's indentation
    if ((Pressed(KEY_ENTER) || Pressed(KEY_KP_ENTER)) && !ctrl) {
        Checkpoint(EditKind::Other);
        Insert("\n" + IndentBefore(HasSelection() ? SelStart() : cursor));
        edited = true;
    }

    // Tab: insert a tab, or indent the selected lines; Shift+Tab: unindent
    if (Pressed(KEY_TAB)) {
        bool multiLine = HasSelection() && text.find('\n', SelStart()) < SelEnd();
        Checkpoint(EditKind::Other);
        if (shift) UnindentLines();
        else if (multiLine) IndentLines();
        else Insert("\t");
        edited = true;
    }

    // Undo: Ctrl/Cmd+Z; redo: Ctrl/Cmd+Shift+Z or Ctrl/Cmd+Y
    if (ctrl && Pressed(KEY_Z)) {
        if (shift) Redo(); else Undo();
        edited = true;
    }
    if (ctrl && Pressed(KEY_Y)) { Redo(); edited = true; }

    // Clipboard and select-all
    if (ctrl && IsKeyPressed(KEY_A)) { anchor = 0; cursor = text.size(); BreakUndoGroup(); }
    if (ctrl && (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_X)) && HasSelection()) {
        SetClipboardText(text.substr(SelStart(), SelEnd() - SelStart()).c_str());
        if (IsKeyPressed(KEY_X)) { Checkpoint(EditKind::Other); DeleteSelection(); edited = true; }
    }
    if (ctrl && IsKeyPressed(KEY_V)) {
        std::string clip = ClipboardText(true);
        if (!clip.empty() || HasSelection()) { Checkpoint(EditKind::Other); Insert(clip); }
        edited = true;
    }

    // Deletion
    if (Pressed(KEY_BACKSPACE)) {
        if (HasSelection()) { Checkpoint(EditKind::Other); DeleteSelection(); }
        else if (cursor > 0) { Checkpoint(EditKind::Deleting); text.erase(--cursor, 1); anchor = cursor; }
        edited = true;
    }
    if (Pressed(KEY_DELETE)) {
        if (HasSelection()) { Checkpoint(EditKind::Other); DeleteSelection(); }
        else if (cursor < text.size()) { Checkpoint(EditKind::Deleting); text.erase(cursor, 1); }
        edited = true;
    }

    // Navigation (Shift extends the selection); moving the cursor ends a typing group
    if (Pressed(KEY_LEFT) || Pressed(KEY_RIGHT) || Pressed(KEY_UP) || Pressed(KEY_DOWN) ||
        IsKeyPressed(KEY_HOME) || IsKeyPressed(KEY_END))
        BreakUndoGroup();
    if (Pressed(KEY_LEFT)) {
        if (HasSelection() && !shift) MoveCursor(SelStart(), false);
        else if (cursor > 0) MoveCursor(cursor - 1, shift);
        edited = true;
    }
    if (Pressed(KEY_RIGHT)) {
        if (HasSelection() && !shift) MoveCursor(SelEnd(), false);
        else MoveCursor(cursor + 1, shift);
        edited = true;
    }
    if (Pressed(KEY_UP))   { MoveVertical(-1, shift); edited = true; }
    if (Pressed(KEY_DOWN)) { MoveVertical(+1, shift); edited = true; }
    if (IsKeyPressed(KEY_HOME)) { MoveCursor(LineStart(cursor), shift); edited = true; }
    if (IsKeyPressed(KEY_END))  { MoveCursor(LineEnd(cursor), shift); edited = true; }

    if (edited) ScrollToCursor();
}

EditorAction TreeEditor::Update() {
    // The buttons work in both states; clicking Load/Save again switches the prompt
    if (Clicked(GenerateButton())) { prompt = PromptMode::None; return EditorAction::Generate; }
    if (Clicked(LoadButton()))     { OpenPrompt(PromptMode::Load); return EditorAction::None; }
    if (Clicked(SaveButton()))     { OpenPrompt(PromptMode::Save); return EditorAction::None; }
    if (Clicked(RandomButton()))   { OpenPrompt(PromptMode::Random); return EditorAction::None; }

    for (int i = 0; i < kInsertCount; i++) {
        if (Clicked(InsertButton(i))) {
            Checkpoint(EditKind::Other);
            std::string snippet = kInsertItems[i].snippet;
            size_t token = snippet.find(kColorToken);
            if (token != std::string::npos)
                snippet.replace(token, std::string(kColorToken).size(), RandomColor());
            InsertSnippet(snippet);
            ScrollToCursor();
            return EditorAction::None;
        }
    }

    if (PromptOpen()) return UpdatePrompt();

    if (CtrlDown() && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))) return EditorAction::Generate;
    if (CtrlDown() && IsKeyPressed(KEY_S)) { OpenPrompt(PromptMode::Save); return EditorAction::None; }
    if (CtrlDown() && IsKeyPressed(KEY_O)) { OpenPrompt(PromptMode::Load); return EditorAction::None; }

    UpdateTextBox();
    return EditorAction::None;
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
        float x = box.x + kTextInsetX;
        float y = box.y + kTextInsetY - scrollY;
        size_t selS = SelStart(), selE = SelEnd();
        float spaceW = Measure(" ", kFontSize);
        size_t start = 0;

        while (true) {
            size_t end = LineEnd(start);
            bool visible = y + kLineHeight >= box.y && y <= box.y + box.height;

            if (visible) {
                // Selection highlight (a selected line break shows as one space)
                if (HasSelection() && selS <= end && selE > start) {
                    size_t a = std::max(selS, start), b = std::min(selE, end);
                    float x0 = x + XOf(start, a);
                    float x1 = x + XOf(start, b) + (selE > end ? spaceW : 0);
                    DrawRectangleRec({ x0, y - 1, x1 - x0, kLineHeight }, Fade(SKYBLUE, 0.6f));
                }

                std::string line = ExpandTabs(text.substr(start, end - start));
                size_t first = line.find_first_not_of(' ');
                bool comment = first != std::string::npos && line[first] == '#';
                Text(line, x, y, kFontSize, comment ? GRAY : BLACK);

                // Blinking cursor (hidden while the file prompt has the keyboard)
                if (focused && !PromptOpen() && cursor >= start && cursor <= end && blinkOn) {
                    float cx = x + XOf(start, cursor);
                    DrawRectangleRec({ cx, y - 1, 2, kFontSize + 2 }, MAROON);
                }
            }

            if (end == text.size()) break;
            start = end + 1;
            y += kLineHeight;
        }
    }
    EndScissorMode();

    // Insert-button column
    Text("Insert", InsertButton(0).x, panel.y + kPad, kTitleSize, DARKGRAY);
    for (int i = 0; i < kInsertCount; i++)
        DrawButton(InsertButton(i), kInsertItems[i].label, true);

    // Buttons
    DrawButton(GenerateButton(), "Generate", true);
    DrawButton(LoadButton(), "Load...", true);
    DrawButton(SaveButton(), "Save as...", true);
    DrawButton(RandomButton(), "Random...", true);

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
