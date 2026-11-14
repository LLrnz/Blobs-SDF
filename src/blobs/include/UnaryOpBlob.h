#pragma once
#include "Blob.h"
#include <memory>
#include "../../include/MoreMath.h"

// Unary operators deform space: Evaluate(p) returns child->Evaluate(q), where q is
// p mapped back by the inverse deformation (so the child's shape is deformed forward).
class UnaryOpBlob: public Node{
    protected:
        std::unique_ptr<Node> child;
        UnaryOpBlob(std::unique_ptr<Node> child)
        : child(std::move(child)) {}
};


class TranslateBlob: public UnaryOpBlob{
    private:
        Vector t;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        TranslateBlob(Vector t, std::unique_ptr<Node> child);
};


class RotateXBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateXBlob(double degrees, std::unique_ptr<Node> child);
};

class RotateYBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateYBlob(double degrees, std::unique_ptr<Node> child);
};

class RotateZBlob: public UnaryOpBlob{
    private:
        const double angle;   // radians
        const Matrix33 rot;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        RotateZBlob(double degrees, std::unique_ptr<Node> child);
};

 
class ScaleBlob: public UnaryOpBlob{
    private:
        Vector s;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        ScaleBlob(Vector s, std::unique_ptr<Node> child);
};


class TwistBlob: public UnaryOpBlob{
    private:
        double rate;    // radians per unit of y
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        TwistBlob(double degreesPerUnit, std::unique_ptr<Node> child);
};


class TaperBlob: public UnaryOpBlob{
    private:
        double rate;
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        TaperBlob(double rate, std::unique_ptr<Node> child);
};


class BendBlob: public UnaryOpBlob{
    private:
        double rate;    // radians per unit of x
    public:
        float Evaluate(const Vector& p);
        float GetLipschitzBound();
        void  ComputeBBox();
        BendBlob(double degreesPerUnit, std::unique_ptr<Node> child);
};
