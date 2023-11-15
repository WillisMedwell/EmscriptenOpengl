#version 330 core

layout(location = 0) in vec3 a_position; // The vertex position
layout(location = 1) in vec3 a_norm; // The normal position
layout(location = 2) in vec2 a_uv; // The uv coord

out vec3 v_position;
out vec3 v_norm;
out vec2 v_uv;

void main() {
    v_position = a_position;
    v_norm = a_norm;
    v_uv = a_uv;
}
