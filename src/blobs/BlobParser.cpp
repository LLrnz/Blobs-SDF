#include "include/BlobParser.h"
#include "include/Blob.h"
#include "include/BinaryOpBlob.h"
#include "include/UnaryOpBlob.h"

#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace {

class Parser {
public:
    explicit Parser(const std::string& src) : s(src) {}

    std::unique_ptr<Node> ParseNode() {
        std::string name = Identifier();
        if (name == "node") {
            pos -= 4;   // point the error at the placeholder itself
            Error("replace the NODE placeholder with a primitive or an operator");
        }
        Expect('(');

        std::unique_ptr<Node> node;
        if (name == "sphere") {
            Vector c = Vec();
            float potential = Num();
            node = std::make_unique<SphereBlob>(c, potential);
        }
        else if (name == "capsule") {
            Vector a = Vec();
            Vector b = Vec();
            float potential = Num();
            node = std::make_unique<CapsuleBlob>(a, b, potential);
        }
        else if (IsOperator(name)) {
            std::vector<std::unique_ptr<Node>> children;
            do {
                children.push_back(ParseNode());
                if (!Peek(')') && pos >= s.size())
                    Error("missing ')' to close " + name);
            } while (!Peek(')'));

            if (children.size() < 2)
                Error(name + " needs at least two children");

            // Fold left: each new operator takes ownership of the previous result
            node = std::move(children[0]);
            for (size_t i = 1; i < children.size(); i++)
                node = MakeOperator(name, std::move(node), std::move(children[i]));
        }
        // Unary operators: parameters first, then exactly one child.
        else if (name == "revolution") {
            node = std::make_unique<RevolutionizeYBlob>(Child(name));
        }
        else if (name == "translate") {
            Vector t = Vec();
            node = std::make_unique<TranslateBlob>(t, Child(name));
        }
        else if (name == "rotatex") {
            double degrees = Num();
            node = std::make_unique<RotateXBlob>(degrees, Child(name));
        }
        else if (name == "rotatey") {
            double degrees = Num();
            node = std::make_unique<RotateYBlob>(degrees, Child(name));
        }
        else if (name == "rotatez") {
            double degrees = Num();
            node = std::make_unique<RotateZBlob>(degrees, Child(name));
        }
        else if (name == "scale") {
            Vector f = Vec();
            if (f[0] == 0.0 || f[1] == 0.0 || f[2] == 0.0)
                Error("scale factors must not be 0");
            node = std::make_unique<ScaleBlob>(f, Child(name));
        }
        else if (name == "twist") {
            double rate = Num();
            node = std::make_unique<TwistYBlob>(rate, Child(name));
        }
        else if (name == "taper") {
            double rate = Num();
            node = std::make_unique<TaperYBlob>(rate, Child(name));
        }
        else if (name == "bend") {
            double rate = Num();
            node = std::make_unique<BendBlob>(rate, Child(name));
        }
        else {
            Error("unknown node '" + name + "'");
        }

        Expect(')');
        return node;
    }

    void ExpectEnd() {
        SkipSpace();
        if (pos < s.size())
            Error("unexpected text after the tree");
    }

private:
    const std::string& s;
    size_t pos = 0;

    // The single child of a unary operator, which must be followed by ')'.
    std::unique_ptr<Node> Child(const std::string& name) {
        std::unique_ptr<Node> child = ParseNode();
        if (!Peek(')')) {
            if (pos >= s.size()) Error("missing ')' to close " + name);
            Error(name + " takes exactly one child");
        }
        return child;
    }

    static bool IsOperator(const std::string& name) {
        return name == "union" || name == "blend" || name == "difference" || name == "intersection";
    }

    static std::unique_ptr<Node> MakeOperator(const std::string& name,
                                              std::unique_ptr<Node> l, std::unique_ptr<Node> r) {
        if (name == "union")      return std::make_unique<UnionBlob>(std::move(l), std::move(r));
        if (name == "blend")      return std::make_unique<BlendBlob>(std::move(l), std::move(r));
        if (name == "difference") return std::make_unique<DifferenceBlob>(std::move(l), std::move(r));
        return std::make_unique<IntersectionBlob>(std::move(l), std::move(r));
    }

    // Skips whitespace, commas and '#' comments.
    void SkipSpace() {
        while (pos < s.size()) {
            char c = s[pos];
            if (std::isspace((unsigned char)c) || c == ',') {
                pos++;
            }
            else if (c == '#') {
                while (pos < s.size() && s[pos] != '\n') pos++;
            }
            else {
                break;
            }
        }
    }

    bool Peek(char c) {
        SkipSpace();
        return pos < s.size() && s[pos] == c;
    }

    void Expect(char c) {
        if (!Peek(c))
            Error(std::string("expected '") + c + "'");
        pos++;
    }

    std::string Identifier() {
        SkipSpace();
        size_t start = pos;
        while (pos < s.size() && (std::isalpha((unsigned char)s[pos]) || s[pos] == '_'))
            pos++;
        if (start == pos)
            Error("expected a node name");

        std::string name = s.substr(start, pos - start);
        for (char& c : name) c = (char)std::tolower((unsigned char)c);
        return name;
    }

    float Num() {
        SkipSpace();
        const char* begin = s.c_str() + pos;
        char* end = nullptr;
        double v = std::strtod(begin, &end);
        if (end == begin)
            Error("expected a number");
        pos += end - begin;
        return (float)v;
    }

    Vector Vec() {
        double x = Num();
        double y = Num();
        double z = Num();
        return Vector(x, y, z);
    }

    [[noreturn]] void Error(const std::string& msg) const {
        int line = 1, col = 1;
        for (size_t i = 0; i < pos && i < s.size(); i++) {
            if (s[i] == '\n') { line++; col = 1; }
            else col++;
        }
        throw std::runtime_error("line " + std::to_string(line) + ", col " + std::to_string(col) + ": " + msg);
    }
};

} // namespace

std::unique_ptr<Node> ParseBlobTree(const std::string& text) {
    Parser parser(text);
    std::unique_ptr<Node> root = parser.ParseNode();
    parser.ExpectEnd();
    return root;
}
