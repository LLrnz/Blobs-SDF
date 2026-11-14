#include "include/OperatorBlob.h"
#include "../base/include/mathematics.h"
#include <csignal>

OperatorBlob::OperatorBlob(Node* left, Node* right)
    : left_child(left), right_child(right) {}


BlendBlob::BlendBlob(Node* left, Node* right) : OperatorBlob(left, right) {}

float BlendBlob::Evaluate(const Vector& p){
    return left_child->Evaluate(p) + right_child->Evaluate(p);
}

float BlendBlob::GetLipschitzBound(){
    throw("not implemented");
}

void  BlendBlob::ComputeBBox(){
    throw("not implemented");

}


UnionBlob::UnionBlob(Node* left, Node* right) : OperatorBlob(left, right) {}

float UnionBlob::Evaluate(const Vector& p){
    return Math::Max(left_child->Evaluate(p), right_child->Evaluate(p));
}

float UnionBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  UnionBlob::ComputeBBox(){
    throw("not implemented");

}


DifferenceBlob::DifferenceBlob(Node* left, Node* right) : OperatorBlob(left, right) {}

float DifferenceBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), -right_child->Evaluate(p));

}

float DifferenceBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  DifferenceBlob::ComputeBBox(){
    throw("not implemented");

}


IntersectionBlob::IntersectionBlob(Node* left, Node* right) : OperatorBlob(left, right) {}

float IntersectionBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), right_child->Evaluate(p));

}

float IntersectionBlob::GetLipschitzBound(){
    throw("not implemented");

}

void  IntersectionBlob::ComputeBBox(){
    throw("not implemented");

}
