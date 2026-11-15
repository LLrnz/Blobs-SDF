#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 color;     // per-vertex color (white if the mesh has none)

uniform mat4 mvpMatrix;
uniform mat4 matModel;

out vec3 v_normal;
out vec4 v_color;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);

    // Transform normals into world space
    v_normal = mat3(transpose(inverse(matModel))) * normal;
    v_color  = color;
}
