#pragma once
#include "../base/include/mathematics.h"

class BBox{
    protected:
        Vector left_up, right_down;

    public:
        BBox(Vector left_up, Vector right_down);
        bool isIn(Vector& p);
};