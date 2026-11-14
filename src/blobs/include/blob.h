#pragma once
#include "../../base/include/mathematics.h"
#include <vector>
#include "../../include/Node.h"

#include <cmath>

const int WYVILL_N = 3;

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

    public:
        Blob(float potential);
        float Evaluate(const Vector& p);
        virtual float Distance(const Vector& p) = 0;
};

class SphereBlob: public Blob{
    protected:
        Vector center;
    
    public:
        SphereBlob(Vector center, float potential);
        float Distance(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
};

class CapsuleBlob: public Blob{
    protected:
        Vector a, b;

    public:
        CapsuleBlob(Vector a, Vector b, float potential);

        float Distance(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();

};

