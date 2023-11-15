#version 300

in vec2 v_uv;

uniform sampler2D u_texture;

void main() {
    vec4 sampledColor = texture(u_texture, v_uv); // Sample the color from the texture
    gl_FragColor = sampledColor;
}