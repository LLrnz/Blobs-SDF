#pragma once
#include "../base/include/mathematics.h"
#include "../include/BBox.h"
#include "raylib.h"


class Node{
    protected:
        BBox bb;        // stored by value: set by ComputeBBox()
        Color color;

    public:
        Node() : bb(Vector(0.0), Vector(0.0)) {}
        // Box from the last ComputeBBox() call (a degenerate box at the origin before that)
        const BBox& GetBBox() const { return bb; }
        virtual ~Node() = default;
        virtual float Evaluate(const Vector& p)=0;
        virtual Color EvaluateColor(const Vector& p)=0;
        virtual float GetLipschitzBound()=0;
        virtual BBox ComputeBBox()=0;
};