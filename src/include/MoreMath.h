#pragma once
#include "../base/include/mathematics.h"

class MoreVector{
    public:
        static float Dot(const Vector&, const Vector&);
        static Vector Cross(const Vector&, const Vector&);
        static Vector DivideEachValue(const Vector& u, const Vector& v);
};

// Vector already provides dot (u * v) and cross (u / v) in mathematics.h.
inline float MoreVector::Dot(const Vector& u, const Vector& v){
    return u * v;
}

inline Vector MoreVector::Cross(const Vector& u, const Vector& v){
    return u / v;
}

inline Vector MoreVector::DivideEachValue(const Vector& u, const Vector& v){
    return Vector(u[0]/v[0], u[1]/v[1], u[2]/v[2]);
}

class Matrix33{
    protected:
        Vector v[3];
            
    public:
    //! Empty 
    Matrix33() {}

    explicit Matrix33(Vector v0, Vector v1, Vector v2);

    // Access members
    Vector& operator[] (int);
    Vector operator[] (int) const;

    // Binary operators
    friend Matrix33 operator+ (const Matrix33&, const Matrix33&);
    friend Matrix33 operator- (const Matrix33&, const Matrix33&);

    friend Matrix33 operator* (const Matrix33&, const Matrix33&);
    friend Vector operator* (const Matrix33&, const Vector&);
    friend Matrix33 operator* (const Matrix33&, double);

    friend Matrix33 operator/ (const Matrix33&, double);

    //friend Vector operator/ (const Matrix&, const Matrix&);

    public:
    static const Matrix33 Null; //!< Null vector.
    static const Matrix33 Identity;
};

inline Matrix33::Matrix33(Vector v0, Vector v1, Vector v2){
    Matrix33::v[0] = v0;
    Matrix33::v[1] = v1;
    Matrix33::v[2] = v2;
}

inline Vector operator*(const Matrix33& a, const Vector& b){
    return Vector(  a[0][0]*b[0] + a[0][1]*b[1] + a[0][2]*b[2],
                    a[1][0]*b[0] + a[1][1]*b[1] + a[1][2]*b[2],
                    a[2][0]*b[0] + a[2][1]*b[1] + a[2][2]*b[2]);
}

//! Gets the i-th coordinate of vector.
inline Vector& Matrix33::operator[] (int i)
{
  return v[i];
}

//! Returns the i-th coordinate of vector.
inline Vector Matrix33::operator[] (int i) const
{
  return v[i];
}