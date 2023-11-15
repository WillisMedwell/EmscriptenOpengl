#version 330 

layout(location = 0) in vec3 a_position; 
layout(location = 1) in vec3 a_norm; 
layout(location = 2) in vec2 a_uv; 

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;


void main() {
    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(a_position, 1.0);
}