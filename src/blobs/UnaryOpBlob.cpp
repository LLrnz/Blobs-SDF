#include "include/UnaryOpBlob.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <sys/syslimits.h>


//TODO: compute cos/sin only once per call
namespace {
constexpr double kDegToRad = M_PI / 180.0;

double MaxRadius(BBox b, int a, int c) {
    double r = 0;
    for (double u : { b.Min()[a], b.Max()[a] })
        for (double v : { b.Min()[c], b.Max()[c] })
            r = std::max(r, std::sqrt(u*u + v*v));
    return r;
}
}

Color UnaryOpBlob::EvaluateColor(const Vector& p){
    return child->EvaluateColor(Transform(p));
}

float UnaryOpBlob::Evaluate(const Vector& p){
    return child->Evaluate(Transform(p));
}



//__________________________

TranslateBlob::TranslateBlob(Vector t, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), t(t) {}


Vector TranslateBlob::Transform(const Vector& p){
    return p-t;
}

float TranslateBlob::GetLipschitzBound(){
    // A translation doesn't change distances.
    return child->GetLipschitzBound();
}

BBox  TranslateBlob::ComputeBBox(){
    return child->ComputeBBox()+t;
}



//___________________________

RotateXBlob::RotateXBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(1, 0, 0), 
                    Vector(0, std::cos(angle), std::sin(angle)), 
                    Vector(0, -std::sin(angle), std::cos(angle)))){
    }


Vector RotateXBlob::Transform(const Vector& p){
    return rot*p;
}

float RotateXBlob::GetLipschitzBound(){
    // A rotation doesn't change distances.
    return child->GetLipschitzBound();
}



//__________________________

BBox  RotateXBlob::ComputeBBox(){
    return child->ComputeBBox().Transformed(rot.Transposed());
}

RotateYBlob::RotateYBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(std::cos(angle), 0, -std::sin(angle)),
                    Vector(0, 1, 0), 
                    Vector(std::sin(angle), 0, std::cos(angle)))){
}

Vector RotateYBlob::Transform(const Vector& p){
    return rot*p;
}

float RotateYBlob::GetLipschitzBound(){
    return child->GetLipschitzBound();
}

BBox  RotateYBlob::ComputeBBox(){
    return child->ComputeBBox().Transformed(rot.Transposed());
}



//__________________________

RotateZBlob::RotateZBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(std::cos(angle), std::sin(angle), 0), 
                    Vector(-std::sin(angle), std::cos(angle), 0),
                    Vector(0, 0, 1))){                
}

Vector RotateZBlob::Transform(const Vector& p){
    return rot*p;
}

float RotateZBlob::GetLipschitzBound(){
    return child->GetLipschitzBound();
}

BBox  RotateZBlob::ComputeBBox(){
    return child->ComputeBBox().Transformed(rot.Transposed());
}



//__________________________

ScaleBlob::ScaleBlob(Vector s, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), s(s) {}


Vector ScaleBlob::Transform(const Vector& p){
    return MoreVector::DivideEachValue(p, s);
}

float ScaleBlob::GetLipschitzBound(){
    // Transform divides by s: distances are stretched by at most 1 / min|s_i|.
    double m = std::min({ std::abs(s[0]), std::abs(s[1]), std::abs(s[2]) });
    return child->GetLipschitzBound() / m;
}

BBox  ScaleBlob::ComputeBBox(){
    BBox b = child->ComputeBBox();
    return BBox::FromCorners(MoreVector::MultEach(b.Min(), s), MoreVector::MultEach(b.Max(), s));
}



//__________________________

TwistYBlob::TwistYBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}


Vector TwistYBlob::Transform(const Vector& p){
    Matrix33 rot = Matrix33(  Vector(std::cos(p[1]*rate), 0, -std::sin(p[1]*rate)),
                    Vector(0, 1, 0), 
                    Vector(std::sin(p[1]*rate), 0, std::cos(p[1]*rate)));

    return rot*p;
}

float TwistYBlob::GetLipschitzBound(){
    double rho = MaxRadius(child->ComputeBBox(), 0, 2);
    return child->GetLipschitzBound() * (1.0 + std::abs(rate) * rho);
}

BBox  TwistYBlob::ComputeBBox(){
    BBox b = child->ComputeBBox();
    double r = 0;
    for (double x : { b.Min()[0], b.Max()[0] })
        for (double z : { b.Min()[2], b.Max()[2] })
            r = std::max(r, std::sqrt(x*x + z*z));
    return BBox(Vector(-r, b.Min()[1], -r), Vector(r, b.Max()[1], r));
}



//__________________________

TaperYBlob::TaperYBlob(double rate, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(rate) {}


Vector TaperYBlob::Transform(const Vector& p){
    Vector scale = Vector(1+p[1]*rate, 1, 1+p[1]*rate);
    return MoreVector::DivideEachValue(p, scale);
}

float TaperYBlob::GetLipschitzBound(){
    BBox b = child->ComputeBBox();
    double kmin = std::min(1 + rate * b.Min()[1], 1 + rate * b.Max()[1]);
    if (kmin <= 0)
        return INFINITY;    // the taper pinches to a point (or flips) inside the shape
    double rho = MaxRadius(b, 0, 2);
    return child->GetLipschitzBound() * (std::max(1.0, 1.0 / kmin) + std::abs(rate) * rho / kmin);
}

BBox  TaperYBlob::ComputeBBox(){
    BBox b = child->ComputeBBox();
    double k0 = 1 + rate * b.Min()[1];
    double k1 = 1 + rate * b.Max()[1];

    Vector minest = b.Min(), maxest = b.Max();
    for (int axis : { 0, 2 }) {
        double c[4] = { k0 * b.Min()[axis], k0 * b.Max()[axis],
                        k1 * b.Min()[axis], k1 * b.Max()[axis] };
        minest[axis] = std::min(std::min(c[0], c[1]), std::min(c[2], c[3]));
        maxest[axis] = std::max(std::max(c[0], c[1]), std::max(c[2], c[3]));
    }
    return BBox(minest, maxest);
}



//__________________________

BendBlob::BendBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}

Vector BendBlob::Transform(const Vector& p){

    double angle = rate * p[0];
    double c = std::cos(angle), s = std::sin(angle);
    return Vector(c * p[0] + s * p[1],
                 -s * p[0] + c * p[1],
                  p[2]);
}

float BendBlob::GetLipschitzBound(){
    double rho = MaxRadius(child->ComputeBBox(), 0, 1);
    return child->GetLipschitzBound() * (1.0 + std::abs(rate) * rho);
}

BBox  BendBlob::ComputeBBox(){
    BBox b = child->ComputeBBox();
    double r = 0;
    for (double x : { b.Min()[0], b.Max()[0] })
        for (double y : { b.Min()[1], b.Max()[1] })
            r = std::max(r, std::sqrt(x*x + y*y));
    return BBox(Vector(-r, -r, b.Min()[2]), Vector(r, r, b.Max()[2]));
}


//__________________________
//hyper stylé de le faire en coordonées locales
//opérateur unaire pour ça ?
RevolutionizeYBlob::RevolutionizeYBlob(std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)) {}

Vector RevolutionizeYBlob::Transform(const Vector& p){
    double dist = std::sqrt(p[0]*p[0] + p[2]*p[2]);
    Vector t = Vector(dist, p[1], 0);
    return t;
}

float RevolutionizeYBlob::GetLipschitzBound(){
    // Transform = (distance to the y axis, y, 0). A distance is 1-Lipschitz, so the
    // map doesn't stretch distances.
    return child->GetLipschitzBound();
}

BBox  RevolutionizeYBlob::ComputeBBox(){
    BBox b = child->ComputeBBox();
    double r = std::max(0.0, b.Max()[0]);
    return BBox(Vector(-r, b.Min()[1], -r), Vector(r, b.Max()[1], r));
}


// LocalCoorBlob::LocalCoorBlob(std::unique_ptr<Node> child)
//     : UnaryOpBlob(std::move(child)) {}

// float LocalCoorBlob::Evaluate(const Vector& p){
//     Vector normalized = p;
//     Normalize(normalized);
//     double angle = normalized*Vector(1,0,0);
//     Vector t = Vector(p[0]*angle, p[1], p[2]*angle);
//     return child->Evaluate(t);
// }

// float LocalCoorBlob::GetLipschitzBound(){
//     throw("not implemented");
// }

// void  LocalCoorBlob::ComputeBBox(){
//     throw("not implemented");
// }