#include "include/Blob.h"
#include "../base/include/mathematics.h"
#include "../include/MoreMath.h"
#include <algorithm>

Blob::Blob(float potential){
    this->potential = potential;
}

float Blob::Evaluate(const Vector& p){
    float v =0;
    v += wyvill(Distance(p), potential);
    
    return v;
}

SphereBlob::SphereBlob(Vector center,
        float potential):Blob(potential){
            this->center = center;
        }
        

float  SphereBlob::Distance(const Vector& p){

    return SquaredNorm(p-center);
    
}

float  SphereBlob::GetLipschitzBound(){
   return 1; 
}

void  SphereBlob::ComputeBBox(){
    
}


CapsuleBlob::CapsuleBlob(Vector a, Vector b, float potential):Blob(potential){
            this->a = a;
            this->b = b;
        }
        

float  CapsuleBlob::Distance(const Vector& p){

    Vector pa = p - a;
    Vector ba = b - a;
    float h = std::clamp( MoreVector::Dot(pa,ba)/MoreVector::Dot(ba,ba), 0.0f, 1.0f );
    return SquaredNorm( pa - ba*h );
}

float  CapsuleBlob::GetLipschitzBound(){
   return 1; 
}

void  CapsuleBlob::ComputeBBox(){
    
}