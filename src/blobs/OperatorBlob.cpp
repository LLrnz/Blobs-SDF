#include "include/OperatorBlob.h"
#include "../base/include/mathematics.h"
#include "../base/include/implicits.h"
#include <csignal>
#include <memory>

BlendBlob::BlendBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
    : OperatorBlob(std::move(left), std::move(right)) {}

float BlendBlob::Evaluate(const Vector& p){
    return left_child->Evaluate(p) + right_child->Evaluate(p);
}

float BlendBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  BlendBlob::ComputeBBox(){
    throw("not implemented");

}


UnionBlob::UnionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : OperatorBlob(std::move(left), std::move(right)) {}

float UnionBlob::Evaluate(const Vector& p){
    return Math::Max(left_child->Evaluate(p), right_child->Evaluate(p));
}

float UnionBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  UnionBlob::ComputeBBox(){
    throw("not implemented");

}


DifferenceBlob::DifferenceBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : OperatorBlob(std::move(left), std::move(right)) {}

float DifferenceBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), 2* AnalyticScalarField::threshold - right_child->Evaluate(p));

}

float DifferenceBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  DifferenceBlob::ComputeBBox(){
    throw("not implemented");

}


IntersectionBlob::IntersectionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : OperatorBlob(std::move(left), std::move(right)) {}

float IntersectionBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), right_child->Evaluate(p));

}

float IntersectionBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  IntersectionBlob::ComputeBBox(){
    throw("not implemented");

}
