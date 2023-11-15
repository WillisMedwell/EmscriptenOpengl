#version 330 core

in vec3 v_normal;

void main()
{
    vec3 norm = normalize(v_normal);
    vec3 colour = norm * 0.5 + 0.5;
    gl_FragColor= vec4(colour, 1.0);
}