#pragma once
#include "Blob.h"

class OperatorBlob: public Node{
    protected:
        Node *left_child, *right_child;
        OperatorBlob(Node* left, Node* right);
};

class BlendBlob: public OperatorBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        BlendBlob(Node* left, Node* right);

};

class UnionBlob: public OperatorBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        UnionBlob(Node* left, Node* right);
};

class DifferenceBlob: public OperatorBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        DifferenceBlob(Node* left, Node* right);
};

class IntersectionBlob: public OperatorBlob{
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        IntersectionBlob(Node* left, Node* right);
};