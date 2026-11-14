#pragma once
#include "../base/include/mathematics.h"
#include "../include/BBox.h"


class Node{
    protected:
        BBox bb;

    public:
        Node() : bb(Vector(0.0), Vector(0.0)) {}
        virtual ~Node() = default;
        virtual float Evaluate(const Vector& p)=0;
        virtual float GetLipschitzBound()=0;
        virtual void  ComputeBBox()=0;
};