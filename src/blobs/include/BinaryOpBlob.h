#pragma once
#include "Blob.h"
#include <memory>

class BinaryOpBlob: public Node{
    protected:
        std::unique_ptr<Node> left_child, right_child;
        BinaryOpBlob(std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : left_child(std::move(l)), right_child(std::move(r)) {}
};

class BlendBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        BlendBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);

};

class UnionBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        UnionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
};

class DifferenceBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        DifferenceBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
};

class IntersectionBlob: public BinaryOpBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        IntersectionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right);
};