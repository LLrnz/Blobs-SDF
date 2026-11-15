#version 330

in vec3 v_normal;
in vec4 v_color;     // interpolated vertex color

out vec4 fragment_color;

uniform vec4 colDiffuse;    // material tint, multiplied with the vertex color

// Key light: a directional light coming from the front (+z, where the camera
// starts), slightly above and to the right. This is the direction pointing
// *towards* the light, in world space.
const vec3  keyDir       = normalize(vec3(0.6, 0.8, 1.0));
const float keyIntensity = 0.75;

// Ambient light: a constant term so faces turned away from the key light
// (the back and the underside) are still visible instead of black.
const float ambient = 0.25;

void main()
{
    vec3 N = normalize(v_normal);

    float diffuse = max(dot(N, keyDir), 0.0);
    float light   = ambient + keyIntensity * diffuse;

    vec4 base = v_color * colDiffuse;
    fragment_color = vec4(base.rgb * light, base.a);
}
