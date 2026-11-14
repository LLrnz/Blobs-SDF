#pragma once
#include "../base/include/mathematics.h"

class MoreVector{
    public:
        static float Dot(const Vector&, const Vector&);
        static Vector Cross(const Vector&, const Vector&);
};

// Vector already provides dot (u * v) and cross (u / v) in mathematics.h.
inline float MoreVector::Dot(const Vector& u, const Vector& v){
    return u * v;
}

inline Vector MoreVector::Cross(const Vector& u, const Vector& v){
    return u / v;
}
