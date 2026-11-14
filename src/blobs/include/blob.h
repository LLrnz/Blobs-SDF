#pragma once
#include "../../base/include/mathematics.h"
#include <vector>
#include "../../include/Node.h"

#include <cmath>

inline double gaussian(double x, double sigma)
{
    const double variance = sigma * sigma;
    constexpr double pi = 3.14159265358979323846;

    return std::exp(-x * x / (2.0 * variance))
           / std::sqrt(2.0 * pi * variance);
}

inline double wyvill(double dist_sqrd, int n)
{
    return std::pow((1-dist_sqrd), n);
}

class Blob: public Node{
    protected:
        float potential;
        float threshold;

    public:
        Blob(float potential, float threshold);
        float Evaluate(const Vector& p);
        virtual float Distance(const Vector& p) = 0;
};

class SphereBlob: public Blob{
    protected:
        Vector center;
    
    public:
        SphereBlob(Vector center, float potential, float threshold);
        float Distance(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
};

class CapsuleBlob: public Blob{
    protected:
        Vector a, b;
        float potential;
        float threshold;

    public:
        CapsuleBlob(Vector a, Vector b, float potential, float threshold);

        float Distance(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();

};

