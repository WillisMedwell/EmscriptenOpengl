#version 330 core

precision highp float;
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 v_position[];
in vec3 v_norm[];
in vec2 v_uv[];

uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main() {
    mat4 mvp = u_projection_matrix * u_view_matrix * u_model_matrix;

    for (int i = 0; i < 3; ++i) {
        gl_Position = mvp * vec4(v_position[i], 1.0);
        EmitVertex();

        gl_Position = mvp * vec4(v_position[(i + 1) % 3], 1.0);
        EmitVertex();

        EndPrimitive();
    }
}
