#pragma once
#include "../../include/Node.h"
#include <memory>
#include <vector>


class BlobTree{
    public:

        static std::unique_ptr<Node> SortTree(std::unique_ptr<Node> root);
        static std::vector<Node*> ClosestNeighbor(std::vector<Node*>& nodes);
};