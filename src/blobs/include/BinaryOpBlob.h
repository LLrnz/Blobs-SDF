#pragma once
#include "Blob.h"
#include <memory>

class BinaryOpBlob: public Node{
    protected:
        std::unique_ptr<Node> left_child, right_child;
        BinaryOpBlob(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left_child(std::move(l)), right_child(std::move(r)) {}

    public:
        std::unique_ptr<Node> ReleaseLeft()  { return std::move(left_child); }
        std::unique_ptr<Node> ReleaseRight() { return std::move(right_child); }
        Node* Left() const  { return left_child.get(); }
        Node* Right() const { return right_child.get(); }
};

class BlendBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        BBox      ComputeBBox();
        BlendBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        Color EvaluateColor(const Vector& p);

};

class UnionBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        BBox    ComputeBBox();
        UnionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        Color EvaluateColor(const Vector& p);
};

class DifferenceBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        BBox    ComputeBBox();
        DifferenceBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        Color EvaluateColor(const Vector& p);
};

class IntersectionBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        BBox    ComputeBBox();
        IntersectionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        Color EvaluateColor(const Vector& p);
};