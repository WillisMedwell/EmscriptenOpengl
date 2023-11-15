#version 300
precision highp float;
out vec4 out_colour;

uniform sampler2D u_screen_texture; // The original scene texture
uniform float u_time;
float u_grain_intensity = 0.05; 

in vec2 v_uv; // Texture coordinates

// Function to generate simple noise
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}


vec4 linearToSRGBA(vec4 linear) {
    vec3 srgba;
    for(int i = 0; i < 3; ++i) {
        if (linear[i] <= 0.0031308)
            srgba[i] = 12.92 * linear[i];
        else
            srgba[i] = 1.055 * pow(linear[i], 1.0 / 2.4) - 0.055;
    }
    return vec4(srgba, 1);
    // return pow(srgb, vec4(vec3(1 / 2.2), 1));
}

void main() {
    vec4 base_colour = texture(u_screen_texture, v_uv);

    // gamma corrected.
    vec4 gamma_colour = linearToSRGBA(base_colour);

    // grain
    vec4 grain_colour = vec4(vec3(rand(v_uv * u_time) * u_grain_intensity), float(0.0));

    // vignette
    vec2 center_coord = vec2(textureSize(u_screen_texture, 0) / 2);
    float max_distance_to_center = length(vec2(center_coord));
    float distance_to_center = length(vec2(center_coord) - vec2(gl_FragCoord.xy));
    float distance_to_center_normalised = float(1) - distance_to_center / max_distance_to_center;
    float remapped_distance = float(1.0) - pow(float(10000.0f), float(-distance_to_center_normalised));
    vec4 vignette_scale = vec4(vec3(remapped_distance), 1.0f);

    out_colour = clamp((gamma_colour - grain_colour), 0.0f, 1.0f) * vignette_scale;
}
