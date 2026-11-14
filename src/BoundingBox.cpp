#include "include/BBox.h"

BBox::BBox(Vector left_up, Vector right_down)
    : left_up(left_up), right_down(right_down) {}

bool BBox::isIn(Vector& p){
    for (int i = 0; i < 3; i++){
        if (p[i] < left_up[i] || p[i] > right_down[i])
            return false;
    }
    return true;
}
