#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../../include/Node.h"

// A parsed blob tree. `nodes` owns every node; `root` points into it.
struct BlobTree {
    std::vector<std::unique_ptr<Node>> nodes;
    Node* root = nullptr;
};

// Parses a tree written as nested calls, for example:
//
//   union(
//     capsule(1 0 0, 1 1.5 0, 0.7, 0.5),
//     sphere(0 1 0, 0.7, 0.5)
//   )
//
// Leaves:    sphere(cx cy cz, potential, threshold)
//            capsule(ax ay az, bx by bz, potential, threshold)
// Operators: union, blend, difference, intersection, each taking two or more
//            children; extra children fold left: union(a, b, c) = union(union(a, b), c).
// Commas are optional separators, and '#' starts a comment that runs to the end of the line.
//
// Throws std::runtime_error("line L, col C: ...") on invalid input.
BlobTree ParseBlobTree(const std::string& text);
