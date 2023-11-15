#version 330 

struct PointLight {
    vec3 position;
    vec3 colour;
    float intensity;

    float constant;
    float linear;
    float quadratic;

    // Optional
    float ambient_coefficient;
    float specular_exponent;
};
#define MAX_POINT_LIGHTS 10

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_norm;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform float u_point_lights_size;

out vec4 v_pos;
out vec4 v_normal;

void main() {

    v_pos = u_model_matrix * vec4(a_position, 1.0);
    v_normal = normalize(transpose(inverse(u_model_matrix)) * vec4(a_norm, 0.0));

    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(a_position, 1.0);
}


