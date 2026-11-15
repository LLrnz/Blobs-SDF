#include "include/BinaryOpBlob.h"
#include "../base/include/mathematics.h"
#include "../base/include/implicits.h"
#include <csignal>
#include <memory>
#include <algorithm>
#include "../include/MoreMath.h"

BlendBlob::BlendBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
    : BinaryOpBlob(std::move(left), std::move(right)) {}

float BlendBlob::Evaluate(const Vector& p){
    return left_child->Evaluate(p) + right_child->Evaluate(p);
}

float BlendBlob::GetLipschitzBound(){
    return left_child->GetLipschitzBound() + right_child->GetLipschitzBound();
}

BBox  BlendBlob::ComputeBBox(){
    return BBox::Union(left_child->ComputeBBox(), right_child->ComputeBBox());

}

Color BlendBlob::EvaluateColor(const Vector& p){
    double v1 = left_child->Evaluate(p);
    double v2 = right_child->Evaluate(p);
    double t = v2 / std::max((v1+v2), 0.001);
    return MoreColor::Lerp(t, left_child->EvaluateColor(p), right_child->EvaluateColor(p));
}


UnionBlob::UnionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float UnionBlob::Evaluate(const Vector& p){
    return Math::Max(left_child->Evaluate(p), right_child->Evaluate(p));
}

float UnionBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  UnionBlob::ComputeBBox(){
    return BBox::Union(left_child->ComputeBBox(), right_child->ComputeBBox());


}

Color UnionBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) > right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}


DifferenceBlob::DifferenceBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float DifferenceBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), 2* AnalyticScalarField::threshold - right_child->Evaluate(p));

}

float DifferenceBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  DifferenceBlob::ComputeBBox(){
    return BBox::Difference(left_child->ComputeBBox(), right_child->ComputeBBox());

}

Color DifferenceBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) < 2* AnalyticScalarField::threshold - right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}


IntersectionBlob::IntersectionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float IntersectionBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), right_child->Evaluate(p));

}

float IntersectionBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  IntersectionBlob::ComputeBBox(){
    return BBox::Intersection(left_child->ComputeBBox(), right_child->ComputeBBox());
}

Color IntersectionBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) <= right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}
