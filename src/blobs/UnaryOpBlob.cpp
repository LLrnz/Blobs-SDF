#include "include/UnaryOpBlob.h"
#include <functional>
#include "../base/include/implicits.h"


//TODO: compute cos/sin only once per call
namespace {
constexpr double kDegToRad = M_PI / 180.0;
}

TranslateBlob::TranslateBlob(Vector t, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), t(t) {}

float TranslateBlob::Evaluate(const Vector& p){
    return child->Evaluate(p-t);
}

float TranslateBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  TranslateBlob::ComputeBBox(){
    throw("not implemented");
}


RotateXBlob::RotateXBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(1, 0, 0), 
                    Vector(0, std::cos(angle), std::sin(angle)), 
                    Vector(0, -std::sin(angle), std::cos(angle)))){
    }

float RotateXBlob::Evaluate(const Vector& p){
    return child->Evaluate(rot*p);
}

float RotateXBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  RotateXBlob::ComputeBBox(){
    throw("not implemented");
}

RotateYBlob::RotateYBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(std::cos(angle), 0, -std::sin(angle)),
                    Vector(0, 1, 0), 
                    Vector(std::sin(angle), 0, std::cos(angle)))){
}

float RotateYBlob::Evaluate(const Vector& p){
    return child->Evaluate(rot*p);
}

float RotateYBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  RotateYBlob::ComputeBBox(){
    throw("not implemented");
}

RotateZBlob::RotateZBlob(double degrees, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), angle(degrees * kDegToRad),
     rot(Matrix33(  Vector(std::cos(angle), std::sin(angle), 0), 
                    Vector(-std::sin(angle), std::cos(angle), 0),
                    Vector(0, 0, 1))){                
}

float RotateZBlob::Evaluate(const Vector& p){
    return child->Evaluate(rot*p);
}

float RotateZBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  RotateZBlob::ComputeBBox(){
    throw("not implemented");
}



ScaleBlob::ScaleBlob(Vector s, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), s(s) {}

float ScaleBlob::Evaluate(const Vector& p){
    return child->Evaluate(MoreVector::DivideEachValue(p, s));
}

float ScaleBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  ScaleBlob::ComputeBBox(){
    throw("not implemented");
}


TwistYBlob::TwistYBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}

float TwistYBlob::Evaluate(const Vector& p){
    Matrix33 rot = Matrix33(  Vector(std::cos(p[1]*rate), 0, -std::sin(p[1]*rate)),
                    Vector(0, 1, 0), 
                    Vector(std::sin(p[1]*rate), 0, std::cos(p[1]*rate)));
    return child->Evaluate(rot*p);
}

float TwistYBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  TwistYBlob::ComputeBBox(){
    throw("not implemented");
}


TaperYBlob::TaperYBlob(double rate, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(rate) {}

float TaperYBlob::Evaluate(const Vector& p){
    Vector scale = Vector(1+p[1]*rate, 1, 1+p[1]*rate);
    return child->Evaluate(MoreVector::DivideEachValue(p, scale));
}

float TaperYBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  TaperYBlob::ComputeBBox(){
    throw("not implemented");
}


BendBlob::BendBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}

float BendBlob::Evaluate(const Vector& p){
    Vector t = Vector(0, p[1]*rate, 0);
    return child->Evaluate(p-t);
}

float BendBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  BendBlob::ComputeBBox(){
    throw("not implemented");
}

//hyper stylé de le faire en coordonées locales
//opérateur unaire pour ça ?
RevolutionizeYBlob::RevolutionizeYBlob(std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)) {}

float RevolutionizeYBlob::Evaluate(const Vector& p){
    double dist = std::sqrt(p[0]*p[0] + p[2]*p[2]);
    Vector t = Vector(dist, p[1], 0);
    return child->Evaluate(t);
}

float RevolutionizeYBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  RevolutionizeYBlob::ComputeBBox(){
    throw("not implemented");
}


SymmetryXBlob::SymmetryXBlob(std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)) {}

float SymmetryXBlob::Evaluate(const Vector& p){
    Vector normalized = p;
    Normalize(normalized);
    double angle = normalized*Vector(1,0,0);
    Vector t = Vector(p[0]*angle, p[1], p[2]*angle);
    return child->Evaluate(t);
}

float SymmetryXBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  SymmetryXBlob::ComputeBBox(){
    throw("not implemented");
}

