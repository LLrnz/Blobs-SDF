#ifndef __Ray__
#define __Ray__

#include "mathematics.h"

// TRay class
class TRay
{
protected:
  Vector c; //!< Origin of the ray.
  Vector n; //!< Direction.
public:
  //! Empty.
  TRay() {}
  explicit TRay(const Vector&, const Vector&);
  //! Empty.
  ~TRay() {}

  TRay Reflect(const Vector&, const Vector&);

  Vector operator()(double) const;

  // Functions to access Vector class components
  Vector Origin() const;
  Vector Direction() const;

  friend std::ostream& operator<<(std::ostream&, const TRay&);
};

/*!
\brief Creates a ray.

The direction should be unit:
\code
TRay ray(Vector(0.0,0.0,0.0),Normalized(Vector(2.0,-1.0,3.0)));
\endcode
\param p Origin.
\param d Direction (should be unit vector).
*/
inline TRay::TRay(const Vector& p, const Vector& d)
{
  c = p;
  n = d;
}

/*!
\brief Return the origin of the ray.
*/
inline Vector TRay::Origin() const
{
  return c;
}

/*!
\brief Return the direction of the ray.
*/
inline Vector TRay::Direction() const
{
  return n;
}

/*!
\brief Computes the location of a vertex along the ray.
\param t Parameter.
*/
inline Vector TRay::operator()(double t) const
{
  return c + t * n;
}

#endif