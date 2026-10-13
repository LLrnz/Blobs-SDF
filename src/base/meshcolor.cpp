#include "include/meshcolor.h"

/*!
\brief Create an empty mesh.
*/
MeshColor::MeshColor()
{
}

/*!
\brief Constructor from a TMesh with color array and indices.
\param m Base mesh.
\param cols Color array.
\param carr Color indexes, should be the same size as TMesh::varray and TMesh::narray.
*/
MeshColor::MeshColor(const TMesh& m, const std::vector<Color>& cols, const std::vector<int>& carr) : TMesh(m), colors(cols), carray(carr)
{
}

/*!
\brief Constructor from a TMesh.
\param m the base mesh
*/
MeshColor::MeshColor(const TMesh& m) : TMesh(m)
{
	colors.resize(vertices.size(), Color(1.0, 1.0, 1.0));
	carray = varray;
}

/*!
\brief Empty.
*/
MeshColor::~MeshColor()
{
}