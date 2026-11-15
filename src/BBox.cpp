#include "include/BBox.h"

BBox::BBox(Vector left_down, Vector right_up)
    : min(left_down), max(right_up) {}

bool BBox::isIn(Vector& p){
    for (int i = 0; i < 3; i++){
        if (p[i] < min[i] || p[i] > max[i])
            return false;
    }
    return true;
}

