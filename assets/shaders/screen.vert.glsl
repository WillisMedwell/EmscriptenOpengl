#version 300
precision highp float;

layout (location = 0) in vec2 a_pos; // Position
layout (location = 1) in vec2 a_uv; // Texture coordinates

out vec2 v_uv;

void main()
{
    v_uv = a_uv;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
