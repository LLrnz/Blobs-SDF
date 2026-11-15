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

float BlendBlob::EvalPruned(const Vector& p){
    return left_child->EvalIfIn(p) + right_child->EvalIfIn(p);
}

float BlendBlob::GetLipschitzBound(){
    return left_child->GetLipschitzBound() + right_child->GetLipschitzBound();
}

BBox  BlendBlob::ComputeBBox(){
    BBox b = BBox::Union(left_child->ComputeBBox(), right_child->ComputeBBox());
    bb = b;
    return bb;

}

Color BlendBlob::EvaluateColor(const Vector& p){
    double v1 = left_child->Evaluate(p);
    double v2 = right_child->Evaluate(p);
    double t = v2 / std::max((v1+v2), 0.001);
    return MoreColor::Lerp(t, left_child->EvaluateColor(p), right_child->EvaluateColor(p));
}

Color BlendBlob::EvalColorIfIn(const Vector& p){
    double v1 = left_child->EvalIfIn(p);
    double v2 = right_child->EvalIfIn(p);
    double t = v2 / std::max((v1+v2), 0.001);
    return MoreColor::Lerp(t, left_child->EvaluateColor(p), right_child->EvaluateColor(p));
}

UnionBlob::UnionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float UnionBlob::Evaluate(const Vector& p){
    return Math::Max(left_child->Evaluate(p), right_child->Evaluate(p));
}

float UnionBlob::EvalPruned(const Vector& p){
    return Math::Max(left_child->EvalIfIn(p), right_child->EvalIfIn(p));
}


float UnionBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  UnionBlob::ComputeBBox(){
    bb = BBox::Union(left_child->ComputeBBox(), right_child->ComputeBBox());
    return bb;


}

Color UnionBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) > right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}

Color UnionBlob::EvalColorIfIn(const Vector& p){
    if(left_child->EvalIfIn(p) > right_child->EvalIfIn(p)){
        return left_child->EvalColorIfIn(p);
    }else{
        return right_child->EvalColorIfIn(p);
    }
}


DifferenceBlob::DifferenceBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float DifferenceBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), 2* AnalyticScalarField::threshold - right_child->Evaluate(p));

}

float DifferenceBlob::EvalPruned(const Vector& p){
    if(!left_child->GetBBox().isIn(p)){
        return 0.0f;
    }else if(!right_child->GetBBox().isIn(p)){
        return Math::Min(left_child->EvalIfIn(p), 2*AnalyticScalarField::threshold);
    }else{
        return Math::Min(left_child->EvalIfIn(p), right_child->EvalIfIn(p));
    }
    
}

float DifferenceBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  DifferenceBlob::ComputeBBox(){
    bb = BBox::Difference(left_child->ComputeBBox(), right_child->ComputeBBox());
    return bb;

}

Color DifferenceBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) < 2* AnalyticScalarField::threshold - right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}

Color DifferenceBlob::EvalColorIfIn(const Vector& p){
    if(left_child->EvalIfIn(p) < 2* AnalyticScalarField::threshold - right_child->EvalIfIn(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}


IntersectionBlob::IntersectionBlob(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : BinaryOpBlob(std::move(left), std::move(right)) {}

float IntersectionBlob::Evaluate(const Vector& p){
    return Math::Min(left_child->Evaluate(p), right_child->Evaluate(p));

}

float IntersectionBlob::EvalPruned(const Vector& p){
    if(!left_child->GetBBox().isIn(p) && !right_child->GetBBox().isIn(p)){
        return 0.0f;
    }else{
        return Math::Min(left_child->EvalIfIn(p), right_child->EvalIfIn(p));
    }
    
}

float IntersectionBlob::GetLipschitzBound(){
    return std::max(left_child->GetLipschitzBound(), right_child->GetLipschitzBound());
}

BBox  IntersectionBlob::ComputeBBox(){
    bb = BBox::Intersection(left_child->ComputeBBox(), right_child->ComputeBBox());
    return bb;
}

Color IntersectionBlob::EvaluateColor(const Vector& p){
    if(left_child->Evaluate(p) <= right_child->Evaluate(p)){
        return left_child->EvaluateColor(p);
    }else{
        return right_child->EvaluateColor(p);
    }
}

Color IntersectionBlob::EvalColorIfIn(const Vector& p){
    if(left_child->Evaluate(p) <= right_child->EvalIfIn(p)){
        return left_child->EvalColorIfIn(p);
    }else{
        return right_child->EvalColorIfIn(p);
    }
}