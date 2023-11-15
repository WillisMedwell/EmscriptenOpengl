#version 300
precision highp float;
out vec4 out_colour;


in vec3 v_normal;

void main()
{
    vec3 norm = normalize(v_normal);
    vec3 colour = norm * 0.5 + 0.5;
    out_colour= vec4(colour, 1.0);
}