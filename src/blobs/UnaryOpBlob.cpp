#include "include/UnaryOpBlob.h"


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
    throw("not implemented");
}

float ScaleBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  ScaleBlob::ComputeBBox(){
    throw("not implemented");
}


TwistBlob::TwistBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}

float TwistBlob::Evaluate(const Vector& p){
    throw("not implemented");
}

float TwistBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  TwistBlob::ComputeBBox(){
    throw("not implemented");
}


TaperBlob::TaperBlob(double rate, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(rate) {}

float TaperBlob::Evaluate(const Vector& p){
    throw("not implemented");
}

float TaperBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  TaperBlob::ComputeBBox(){
    throw("not implemented");
}


BendBlob::BendBlob(double degreesPerUnit, std::unique_ptr<Node> child)
    : UnaryOpBlob(std::move(child)), rate(degreesPerUnit * kDegToRad) {}

float BendBlob::Evaluate(const Vector& p){
    throw("not implemented");
}

float BendBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  BendBlob::ComputeBBox(){
    throw("not implemented");
}
