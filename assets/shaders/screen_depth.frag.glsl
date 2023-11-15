#version 300
precision highp float;
out vec4 out_colour;


uniform sampler2D u_screen_texture;
float u_camera_near_plane = 0.01f;
float u_camera_far_plane = 1000.0f;

in vec2 v_uv;

float LineariseDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * u_camera_near_plane * u_camera_far_plane) / (u_camera_far_plane + u_camera_near_plane - z * (u_camera_far_plane - u_camera_near_plane));
}

void main() {
    float depth = texture(u_screen_texture, v_uv).r; // Get depth value
    float linearised_depth = LineariseDepth(depth) / u_camera_far_plane; // Normalize if needed
    out_colour = vec4(vec3(linearised_depth), 1.0); // Output grayscale depth
}