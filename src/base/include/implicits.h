// Implicits

#pragma once

#include <iostream>

#include "mesh.h"
#include "../../include/Node.h"

class AnalyticScalarField
{
protected:
  Node* root;

public:
  AnalyticScalarField(Node* root);
  virtual double Value(const Vector&) const;
  virtual Vector Gradient(const Vector&) const;
  static inline double threshold = 0.5;

  // Number of calls to Value() (field evaluations of the whole tree), for performance
  // measurements. Reset it before an operation and read it after.
  static inline long long evaluations = 0;

  // Normal
  virtual Vector Normal(const Vector&) const;

  // Dichotomy
  Vector Dichotomy(Vector, Vector, double, double, double, const double& = 1.0e-4) const;

  virtual void Polygonize(int, TMesh&, const Box&, const double& = 1e-4, bool colored=false) const;
protected:
  static const double Epsilon; //!< Epsilon value for partial derivatives
protected:
  static int TriangleTable[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
  static int edgeTable[256];    //!< Array storing straddling edges for every marching cubes configuration.
};