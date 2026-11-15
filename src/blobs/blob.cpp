#include "include/Blob.h"
#include "../base/include/mathematics.h"
#include "../include/MoreMath.h"
#include "raylib.h"
#include <algorithm>

const attenuation_func_type att_func = WYVILL;
const lipschtiz_method_type lip_func = ANALYTIC;
const double SAMPLING_N = 1000;
const double SAFETY_FACTOR = 1.01;

Blob::Blob(float potential, Color col){
    this->potential = potential;
    this->color = col;

        //lipschitz bound of wyvill only
    switch (lip_func) {
        case ANALYTIC:
            static const double x = 1/std::sqrt(2*WYVILL_N - 1);
            static const double Kprim = 2 * WYVILL_N * x * pow((1 - x*x), (WYVILL_N-1));
            this->lip_bound =  Kprim/potential;
            break;
        case COMPOSITION:
            this->lip_bound =  2*WYVILL_N/potential;
            break;
        case SAMPLING:
            double samp_x = 0;
            double step = 1/SAMPLING_N;
            double max_k = 0;   
            for (int i = 0; i<SAMPLING_N; i++){
                samp_x += step;
                double Kprimed = 2 * WYVILL_N * samp_x * pow((1 - samp_x*samp_x), (WYVILL_N-1));
                if(Kprimed > max_k){
                    max_k = Kprimed;
                }   
            }
            this->lip_bound =  max_k*SAFETY_FACTOR/potential;
            break;
    }

}

float Blob::Evaluate(const Vector& p){
    float v;
    switch (att_func) {
        case WYVILL: v = wyvill(Distance(p), potential);
            break;
        case GAUSSIAN: v = gaussian(Distance(p),  potential);
            break;
    };
    
    return v;
}

float Blob::EvalPruned(const Vector& p){
    return Evaluate(p);
}


SphereBlob::SphereBlob(Vector center,
        float potential, Color col):Blob(potential, col){
            this->center = center;            
        }
        

float  SphereBlob::Distance(const Vector& p){

    return SquaredNorm(p-center);
    
}

float  SphereBlob::GetLipschitzBound(){
  
   return lip_bound;
}

BBox  SphereBlob::ComputeBBox(){
    Vector R = Vector(potential, potential, potential);

    bb = BBox(center-R, center+R);
    return bb;
}

Color SphereBlob::EvaluateColor(const Vector& p){
    return color;
}

Color SphereBlob::EvalColorIfIn(const Vector& p){
    return color;
}


CapsuleBlob::CapsuleBlob(Vector a, Vector b, 
    float potential,Color col):Blob(potential, col){
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
   // Distance to a segment is 1-Lipschitz, so the bound is the kernel's, like the sphere
   return lip_bound;
}

BBox CapsuleBlob::ComputeBBox(){
    Vector R = Vector(potential, potential, potential);
    Vector segmin = Vector::Min(a, b);
    Vector segmax = Vector::Max(a, b);

    bb = BBox(segmin-R, segmax+R);
    return bb;
}

Color CapsuleBlob::EvaluateColor(const Vector& p){
    return color;
}

Color CapsuleBlob::EvalColorIfIn(const Vector& p){
    return color;
}