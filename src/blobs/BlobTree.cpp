#include "include/BlobTree.h"
#include "include/BinaryOpBlob.h"
#include "include/UnaryOpBlob.h"
#include <cstddef>
#include <limits>
#include <unordered_map>

namespace {

// Operators whose runs can be regrouped freely (commutative and associative)
enum class RunOp { None, Blend, Union, Intersection };

RunOp KindOf(Node* n) {
    if (dynamic_cast<BlendBlob*>(n))        return RunOp::Blend;
    if (dynamic_cast<UnionBlob*>(n))        return RunOp::Union;
    if (dynamic_cast<IntersectionBlob*>(n)) return RunOp::Intersection;
    return RunOp::None;
}

std::unique_ptr<Node> Make(RunOp op, std::unique_ptr<Node> l, std::unique_ptr<Node> r) {
    switch (op) {
        case RunOp::Blend:        return std::make_unique<BlendBlob>(std::move(l), std::move(r));
        case RunOp::Union:        return std::make_unique<UnionBlob>(std::move(l), std::move(r));
        case RunOp::Intersection: return std::make_unique<IntersectionBlob>(std::move(l), std::move(r));
        default:                  throw("BlobTree: not a regroupable operator");
    }
}

void Gather(std::unique_ptr<Node> n, RunOp op, std::vector<std::unique_ptr<Node>>& operands) {
    if (KindOf(n.get()) == op) {
        auto* bin = static_cast<BinaryOpBlob*>(n.get());
        Gather(bin->ReleaseLeft(), op, operands);
        Gather(bin->ReleaseRight(), op, operands);
    }
    else {
        operands.push_back(BlobTree::SortTree(std::move(n)));
    }
}

std::unique_ptr<Node> Build(std::vector<std::unique_ptr<Node>>& operands, size_t lo, size_t hi, RunOp op) {
    if (hi - lo == 1)
        return std::move(operands[lo]);
    size_t mid = (lo + hi) / 2;
    std::unique_ptr<Node> left = Build(operands, lo, mid, op);
    std::unique_ptr<Node> right = Build(operands, mid, hi, op);
    return Make(op, std::move(left), std::move(right));
}

} 

std::unique_ptr<Node> BlobTree::SortTree(std::unique_ptr<Node> root) {
    Node* n = root.get();


    if (auto* unary = dynamic_cast<UnaryOpBlob*>(n)) {
        unary->SetChild(SortTree(unary->ReleaseChild()));
        return root;
    }

    if (auto* diff = dynamic_cast<DifferenceBlob*>(n)) {
        std::unique_ptr<Node> l = SortTree(diff->ReleaseLeft());
        std::unique_ptr<Node> r = SortTree(diff->ReleaseRight());
        return std::make_unique<DifferenceBlob>(std::move(l), std::move(r));
    }

    RunOp op = KindOf(n);
    if (op == RunOp::None)
        return root;    

  
    std::vector<std::unique_ptr<Node>> operands;
    Gather(std::move(root), op, operands);

    std::vector<Node*> raw;
    for (auto& o : operands) {
        o->ComputeBBox();
        raw.push_back(o.get());
    }
    std::vector<Node*> order = ClosestNeighbor(raw);

    std::unordered_map<Node*, size_t> index;
    for (size_t i = 0; i < raw.size(); i++) index[raw[i]] = i;
    std::vector<std::unique_ptr<Node>> sorted;
    for (Node* o : order) sorted.push_back(std::move(operands[index[o]]));

    return Build(sorted, 0, sorted.size(), op);
}

std::vector<Node*> BlobTree::ClosestNeighbor(std::vector<Node*>& nodes){
    if(nodes.empty()){throw("empty node list");}

    std::unordered_map<Node*, bool> visited;
    for(size_t i = 0; i<nodes.size(); i++){
        visited.insert({nodes[i], false});
    }

    std::vector<Node*> res;
    res.push_back(nodes[0]);
    Node* current_node = nodes[0];

    visited[current_node] = true;

    for(size_t i = 0; i<nodes.size()-1; i++){
        double closest_dist = std::numeric_limits<double>::max();
        Node* closest_node = nullptr;

        for(size_t j = 1; j<nodes.size(); j++){
            Node* neighbor = nodes[j];

            if(!visited[neighbor]){
                double dist = current_node->Bbox()->Distance(neighbor->Bbox());
                if(dist < closest_dist){
                    closest_dist = dist;
                    closest_node = neighbor;
                }
            }
            
        }
        res.push_back(closest_node);
        visited[closest_node] = true;
        current_node = closest_node;
    }
    return res;
}