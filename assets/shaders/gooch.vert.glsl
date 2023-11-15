#version 330 core

layout(location = 0) in vec3 a_position; 
layout(location = 1) in vec3 a_norm; 
layout(location = 2) in vec2 a_uv; 

struct PointLight {
    vec3 position;
    vec3 colour;
    float intensity;

    float constant;
    float linear;
    float quadratic;

    float ambient_coefficient;
    float specular_exponent;
};
#define MAX_POINT_LIGHTS 10

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform float u_point_lights_size;

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

out vec3 v_normal;
out vec3 v_frag_position;

void main() {
    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(a_position, 1.0);

    v_normal = mat3(transpose(inverse(u_model_matrix))) * a_norm;
    v_frag_position = vec3(u_model_matrix * vec4(a_position, 1.0));
}