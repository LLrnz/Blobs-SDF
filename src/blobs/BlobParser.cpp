#include "include/BlobParser.h"
#include "include/Blob.h"
#include "include/OperatorBlob.h"

#include <cctype>
#include <cstdlib>
#include <stdexcept>

namespace {

class Parser {
public:
    Parser(const std::string& src, BlobTree& tree) : s(src), tree(tree) {}

    Node* ParseNode() {
        std::string name = Identifier();
        Expect('(');

        Node* node = nullptr;
        if (name == "sphere") {
            Vector c = Vec();
            float potential = Num();
            node = Make<SphereBlob>(c, potential);
        }
        else if (name == "capsule") {
            Vector a = Vec();
            Vector b = Vec();
            float potential = Num();
            node = Make<CapsuleBlob>(a, b, potential);
        }
        else if (IsOperator(name)) {
            std::vector<Node*> children;
            do {
                children.push_back(ParseNode());
                if (!Peek(')') && pos >= s.size())
                    Error("missing ')' to close " + name);
            } while (!Peek(')'));

            if (children.size() < 2)
                Error(name + " needs at least two children");

            node = children[0];
            for (size_t i = 1; i < children.size(); i++)
                node = MakeOperator(name, node, children[i]);
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
    BlobTree& tree;
    size_t pos = 0;

    template <typename T, typename... Args>
    Node* Make(Args&&... args) {
        tree.nodes.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return tree.nodes.back().get();
    }

    static bool IsOperator(const std::string& name) {
        return name == "union" || name == "blend" || name == "difference" || name == "intersection";
    }

    Node* MakeOperator(const std::string& name, Node* l, Node* r) {
        if (name == "union")      return Make<UnionBlob>(l, r);
        if (name == "blend")      return Make<BlendBlob>(l, r);
        if (name == "difference") return Make<DifferenceBlob>(l, r);
        return Make<IntersectionBlob>(l, r);
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

BlobTree ParseBlobTree(const std::string& text) {
    BlobTree tree;
    Parser parser(text, tree);
    tree.root = parser.ParseNode();
    parser.ExpectEnd();
    return tree;
}
