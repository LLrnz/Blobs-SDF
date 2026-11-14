#version 330

in vec3 v_normal;

out vec4 fragment_color;

uniform vec4 colDiffuse;

void main()
{
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    vec3 N = normalize(v_normal);

    float costheta = max(dot(N, lightDir), 0.0);

    fragment_color = vec4(colDiffuse.rgb * costheta, colDiffuse.a);
}