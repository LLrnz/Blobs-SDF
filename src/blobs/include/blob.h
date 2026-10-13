#include "../../base/include/mathematics.h"
#include <vector>

#include <cmath>

inline double gaussian(double x, double sigma)
{
    const double variance = sigma * sigma;
    constexpr double PI = 3.14159265358979323846;

    return std::exp(-x * x / (2.0 * variance))
           / std::sqrt(2.0 * PI * variance);
}

class Blob{
    protected:
        float threshold;

    public:
        virtual float Value(const Vector& p)=0;
        virtual float Lipschitz()=0;
};

class PointBlob: public Blob{
    protected:
        std::vector<Vector> centers;
        std::vector<float> potentials;
    
    public:
        PointBlob(std::vector<Vector> centers, std::vector<float> potentials, float threshold);
        float Value(const Vector& p);
        float Lipschitz();
};

class SegmentBlob: public Blob{
    protected:
        Vector a, b;
    
    public:
        float Value(const Vector& p);
        float Lipschitz();

};