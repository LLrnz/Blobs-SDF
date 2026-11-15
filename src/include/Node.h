#pragma once
#include "../base/include/mathematics.h"
#include "../include/BBox.h"
#include "raylib.h"


class Node{
    protected:
        //BBox bb;
        Color color;

    public:
        Node(){}// : bb(Vector(0.0), Vector(0.0)) {}
        virtual ~Node() = default;
        virtual float Evaluate(const Vector& p)=0;
        virtual Color EvaluateColor(const Vector& p)=0;
        virtual float GetLipschitzBound()=0;
        virtual BBox ComputeBBox()=0;
};