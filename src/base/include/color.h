// Color

#ifndef __Color__
#define __Color__

// Mathematics fundamentals
#include "mathematics.h"

class TColor
{
protected:
  double c[4]; //!< Array of color components; includes an alpha channel.
public:
  explicit TColor(double = 0.0);
  explicit TColor(unsigned long);
  explicit TColor(double, double, double, double = 1.0);
  explicit TColor(int, int, int, int = 255);

  //! Empty
  ~TColor() {}

  TColor& operator+=(const TColor&);

  TColor Scale(const TColor&) const;

  friend TColor operator+(const TColor&, const TColor&);
  friend TColor operator-(const TColor&, const TColor&);

  friend TColor operator*(const TColor&, double);
  friend TColor operator*(double, const TColor&);
  friend TColor operator/(const TColor&, double);

  static TColor Lerp(double, const TColor&, const TColor&);

  double& operator[] (int);
  double operator[] (int) const;

  static const TColor White; //!< White.
  static const TColor Transparent; //!< Transparent.
};

/*!
\brief Returns the i<sup>th</sup> channel of the spectrum
\param i nummber of the channel queried (default=last)
*/
inline double& TColor::operator[] (int i)
{
  return c[i];
}

/*!
\brief Returns a copy of the i<sup>th</sup> channel of the spectrum
\param i nummber of the channel queried (default=last)
*/
inline double TColor::operator[] (int i) const
{
  return c[i];
}

/*!
\brief Linear interpolation between two colors.

Interpolation is performed in RGB space.

Note that the following lines are equivalent:
\code
Color a,b;
double t;
Color c=(1.0-t)*a+t*b;
Color c=Lerp(t,a,b);
\endcode
*/
inline TColor TColor::Lerp(double t, const TColor& a, const TColor& b)
{
  return TColor((1.0 - t) * a[0] + t * b[0], (1.0 - t) * a[1] + t * b[1], (1.0 - t) * a[2] + t * b[2], (1.0 - t) * a[3] + t * b[3]);
}

/*!
\brief Creates a greyscale color.

Initializes all the components to the given value except the opacity coefficient wich is set to 1.0.
\param v Grey value.
*/
inline TColor::TColor(double v)
{
  c[0] = c[1] = c[2] = v;
  c[3] = 1.0;
}

/*!
\brief Creates a color given a compact color representation
\param x Color compacted into an unsigned long.
*/
inline TColor::TColor(unsigned long x)
{
  c[0] = ((x >> 24) & 255) / 255.0;
  c[1] = ((x >> 16) & 255) / 255.0;
  c[2] = ((x >> 8) & 255) / 255.0;
  c[3] = (x & 255) / 255.0;
}

/*!
\brief Creates a color given each of these components
\param r,g,b Red, green and blue components.
\param a Alpha channel, set to 1.0 (opaque) as default.
*/
inline TColor::TColor(double r, double g, double b, double a)
{
  c[0] = r;
  c[1] = g;
  c[2] = b;
  c[3] = a;
}

/*!
\brief Create a color given integer components
\param r, g, b Red, green and blue components.
\param a Alpha channel, set to 255 (opaque) as default.
*/
inline TColor::TColor(int r, int g, int b, int a) :TColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
{
}

/*!
\brief Overloaded operator.
*/
inline TColor& TColor::operator+=(const TColor& ac)
{
  c[0] += ac[0];
  c[1] += ac[1];
  c[2] += ac[2];
  c[3] += ac[3];
  return *this;
}

/*!
\brief Overloaded sum operator
*/
inline TColor operator+(const TColor& u, const TColor& v)
{
  return TColor(u[0] + v[0], u[1] + v[1], u[2] + v[2], u[3] + v[3]);
}

/*!
\brief Scale.
\param v %Color.
*/
inline TColor TColor::Scale(const TColor& v) const
{
  return TColor(c[0] * v[0], c[1] * v[1], c[2] * v[2], c[3] * v[3]);
}

/*!
\brief Overloaded difference operator
*/
inline TColor operator-(const TColor& u, const TColor& v)
{
  return TColor(u[0] - v[0], u[1] - v[1], u[2] - v[2], u[3] - v[3]);
}

/*!
\brief Overloaded product by a scalar operator.
*/
inline TColor operator*(const TColor& u, double a)
{
  return TColor(u[0] * a, u[1] * a, u[2] * a, u[3] * a);
}

/*!
\brief Overloaded product by a scalar operator

\param c Color.
\param a Scalar.
*/
inline TColor operator*(double a, const TColor& c)
{
  return c * a;
}

/*!
\brief Overloaded division by a scalar operator.
\param c Color.
\param a Scalar.
*/
inline TColor operator/(const TColor& c, double a)
{
  return c * (1.0 / a);
}

#endif