#include "include/blob.h"

PointBlob::PointBlob(std::vector<Vector> centers,
        std::vector<float> potentials, float threshold){
            this->centers = centers;
            this->potentials = potentials;
            if (centers.size() != potentials.size()){
                throw "Must have a potential for every center (must be same size).";
            }
            this->threshold = threshold;
        }
        

float  PointBlob::Value(const Vector& p){
    float v =0;
    for(size_t i = 0; i < centers.size(); i++){
        float dist = Norm(p-centers[i]);
        v += gaussian(dist, potentials[i]);
    }
    v -= threshold;
    return v;
}

float  PointBlob::Lipschitz(){
   return 1; 
}

float  SegmentBlob::Value(const Vector& p){
    return 1;
}

float  SegmentBlob::Lipschitz(){
    return 1;
}