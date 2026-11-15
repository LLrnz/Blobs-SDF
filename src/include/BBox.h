#pragma once
#include "../base/include/mathematics.h"
#include "../include/MoreMath.h"

class BBox{
    protected:
        Vector min, max;

    public:
        BBox(Vector left_down, Vector right_up);
        bool isIn(Vector& p);

        static inline BBox Union(BBox a, BBox b){
            Vector minest = Vector::Min(a.min, b.min);
            Vector maxest = Vector::Max(a.max, b.max);

            return BBox(minest, maxest);
        }
        static inline BBox Intersection(BBox a, BBox b){
            Vector minest = Vector::Max(a.min, b.min);
            Vector maxest = Vector::Min(a.max, b.max);

            return BBox(minest, maxest);
        }
        // Carving b out of a only removes material, so a's box always contains the result.
        static inline BBox Difference(BBox a, BBox /*b*/){
            return a;
        }

        // Box spanned by two opposite corners given in any order (e.g. after a
        // negative scale swapped min and max on some axes).
        static inline BBox FromCorners(Vector a, Vector b){
            return BBox(Vector::Min(a, b), Vector::Max(a, b));
        }

        inline BBox operator+(Vector v){
            return BBox(min+v, max+v);
        }
        inline BBox operator-(Vector v){
            return BBox(min-v, max-v);
        }

        inline BBox operator*(double s){
            return BBox(min*s, max*s);
        }

        inline BBox Transformed(const Matrix33& m) const{
            Vector lo, hi;
            for (int i = 0; i < 8; i++){
                Vector corner((i & 1) ? max[0] : min[0],
                              (i & 2) ? max[1] : min[1],
                              (i & 4) ? max[2] : min[2]);
                Vector q = m * corner;
                if (i == 0){ lo = q; hi = q; }
                lo = Vector::Min(lo, q);
                hi = Vector::Max(hi, q);
            }
            return BBox(lo, hi);
        }

        static inline BBox DivideEach(BBox b, Vector s){
            return BBox(MoreVector::DivideEachValue(b.min, s), MoreVector::DivideEachValue(b.max, s));
        }

        static inline BBox MultEach(BBox b, Vector s){
            return BBox(MoreVector::MultEach(b.min, s), MoreVector::MultEach(b.max, s));
        }

        inline Vector& Min(){
            return min;
        }

        inline Vector& Max(){
            return max;
        }


};