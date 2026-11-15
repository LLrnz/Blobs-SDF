#include "include/BBox.h"
#include "base/include/mathematics.h"

BBox::BBox(Vector left_down, Vector right_up)
    : min(left_down), max(right_up) {}

bool BBox::isIn(const Vector& p) const{
    for (int i = 0; i < 3; i++){
        if (p[i] < min[i] || p[i] > max[i])
            return false;
    }
    return true;
}

double BBox::Distance(BBox* b){
    return Norm(Middle() - b->Middle());  
}

Vector BBox::Middle(){
    return Vector((min[0]+max[0])/2, (min[1]+max[1])/2,(min[2]+max[2])/2);
}