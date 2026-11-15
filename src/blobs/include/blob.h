#pragma once
#include "../../base/include/mathematics.h"
#include <vector>
#include "../../include/Node.h"
#include "raylib.h"

#include <cmath>

const int WYVILL_N = 3;

enum attenuation_func_type{
    WYVILL,
    GAUSSIAN
};

enum lipschtiz_method_type{
    ANALYTIC,
    COMPOSITION,
    SAMPLING,
};


inline double gaussian(double dist_sqrd, double radius)
{
    const double variance = radius * radius;
    constexpr double pi = 3.14159265358979323846;

    return std::exp(-dist_sqrd / (2.0 * variance));
           /// std::sqrt(2.0 * pi * variance);
}

inline double wyvill(double dist_sqrd, double radius)
{
    double val = std::max(0.0, 1.0 - dist_sqrd / (radius * radius));
    return std::pow(val, WYVILL_N);
}

class Blob: public Node{
    protected:
        float potential;
        float lip_bound;

    public:
        Blob(float potential, Color col);
        float Evaluate(const Vector& p);
        float EvalPruned(const Vector& p);
        virtual float Distance(const Vector& p) = 0;
};

class SphereBlob: public Blob{
    protected:
        Vector center;
    
    public:
        SphereBlob(Vector center, float potential, Color col);
        float Distance(const Vector& p);
        Color EvaluateColor(const Vector& p);
        Color EvalColorIfIn(const Vector& p);
        
        float GetLipschitzBound();
        BBox  ComputeBBox();
};

class CapsuleBlob: public Blob{
    protected:
        Vector a, b;

    public:
        CapsuleBlob(Vector a, Vector b, float potential, Color col);

        float Distance(const Vector& p);
        Color EvaluateColor(const Vector& p);
        Color EvalColorIfIn(const Vector& p);
        float GetLipschitzBound();
        BBox  ComputeBBox();

};

// class PlaneBlob: public Blob{
//     protected:
//         Vector c, normal;

//     public:
//         PlaneBlob(Vector c, Vector normal, float potential);

//         float Distance(const Vector& p);
//         float GetLipschitzBound();
//         void  ComputeBBox();

// };

// class TorusBlob: public Blob{
//     protected:
//         Vector c;
//         float r, R;

//     public:
//         TorusBlob(Vector c, float r, float R, float potential);

//         float Distance(const Vector& p);
//         float GetLipschitzBound();
//         void  ComputeBBox();

// };


