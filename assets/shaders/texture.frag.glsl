#version 300

precision highp float;
out vec4 out_colour;

in vec2 v_uv;

uniform sampler2D u_texture;

void main() {
    vec4 sampledColor = texture(u_texture, v_uv); // Sample the color from the texture
    out_colour = sampledColor;
}