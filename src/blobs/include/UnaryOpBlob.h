#pragma once
#include "Blob.h"
#include <memory>
#include "../../include/MoreMath.h"

// Unary operators deform space: Evaluate(p) returns child->Evaluate(q), where q is
// p mapped back by the inverse deformation (so the child's shape is deformed forward).
class UnaryOpBlob: public Node{
    protected:
        std::unique_ptr<Node> child;
        float Evaluate(const Vector& p);
        Color EvaluateColor(const Vector& p);
        virtual Vector Transform(const Vector& p)=0;
        UnaryOpBlob(std::unique_ptr<Node> child)
        : child(std::move(child)) {}
        
};


class TranslateBlob: public UnaryOpBlob{
    private:
        Vector t;
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        TranslateBlob(Vector t, std::unique_ptr<Node> child);
       
};


class RotateXBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateXBlob(double degrees, std::unique_ptr<Node> child);
       
};

class RotateYBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateYBlob(double degrees, std::unique_ptr<Node> child);
       
};

class RotateZBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateZBlob(double degrees, std::unique_ptr<Node> child);
       
};

 
class ScaleBlob: public UnaryOpBlob{
    private:
        Vector s;
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        ScaleBlob(Vector s, std::unique_ptr<Node> child);
       
};


class TwistYBlob: public UnaryOpBlob{
    private:
        double rate;    // radians per unit of y
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        TwistYBlob(double degreesPerUnit, std::unique_ptr<Node> child);
       
};


class TaperYBlob: public UnaryOpBlob{
    private:
        double rate;
    public:
        
        Vector Transform(const Vector& p);

        float GetLipschitzBound();
        void  ComputeBBox();
        TaperYBlob(double rate, std::unique_ptr<Node> child);
       
};


class BendBlob: public UnaryOpBlob{
    private:
        double rate;    // radians per unit of x
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        BendBlob(double degreesPerUnit, std::unique_ptr<Node> child);
       
};

class RevolutionizeYBlob: public UnaryOpBlob{
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RevolutionizeYBlob(std::unique_ptr<Node> child);
       
};

class SymmetryXBlob: public UnaryOpBlob{
    public:
        
        Vector Transform(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        SymmetryXBlob(std::unique_ptr<Node> child);
       
};

// class LocalCoorBlob: public UnaryOpBlob{
//     private:
//         Vector local_origin;
//     public:
//         
//        Vector Transform(const Vector& p);
//         float GetLipschitzBound();
//         void  ComputeBBox();
//         LocalCoorBlob(std::unique_ptr<Node> child);
// };
