#version 300
precision highp float;

uniform sampler2D u_screen_texture;
out vec4 out_colour;


in vec2 v_uv;

void main() {
    out_colour = clamp(texture(u_screen_texture, v_uv), 0.0, 1.0);
}

/*
// pixelation
uniform sampler2D u_screen_texture;

in vec2 v_uv;

void main() {
    vec2 u_resolution = vec2(256, 256);

    // Calculate pixel size based on the desired resolution
    vec2 pixelSize = 1.0 / u_resolution;

    // Adjust the texture coordinates
    vec2 pixelatedUV = floor(v_uv / pixelSize) * pixelSize;

    // Sample the texture using the adjusted coordinates
    out_colour = texture(u_screen_texture, pixelatedUV);
}
*/

/*

uniform sampler2D u_screen_texture; // The original scene texture
uniform float u_time = 1.0f; // Current time for dynamic grain
uniform float u_grain_intensity = 0.05f; // Intensity of the grain effect

in vec2 v_uv; // Texture coordinates

// Function to generate simple noise
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    // Get the original color from the scene
    vec4 originalColor = texture(u_screen_texture, v_uv);

    // Generate grain
    float grain = rand(v_uv * u_time) * u_grain_intensity;

    // Apply the grain to the original color
    vec4 grainColor = originalColor + vec4(grain, grain, grain, 0.0);

    // Output the final color
    out_colour = grainColor;
}
*/

/*
// outline 
in vec2 v_uv; // Change TexCoords to v_uv
uniform sampler2D u_screen_texture; // Input texture from the framebuffer

void main() {
    // Define the Sobel filter kernels
    mat3 sobelX = mat3(-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0);

    mat3 sobelY = mat3(-1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0);

    // Initialize the color accumulators
    vec3 colorX = vec3(0.0);
    vec3 colorY = vec3(0.0);

    // Iterate over the 3x3 kernel region
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            // Calculate the sampled texture coordinates with wrapping
            vec2 sampleCoords = v_uv + vec2(i, j) / textureSize(u_screen_texture, 0);
            sampleCoords = fract(sampleCoords); // Apply wrapping

            // Sample the color from the texture
            vec3 sample = texture(u_screen_texture, sampleCoords).rgb;

            // Update the color accumulators with the filtered values
            colorX += sample * sobelX[i + 1][j + 1];
            colorY += sample * sobelY[i + 1][j + 1];
        }
    }

    // Calculate the magnitude of the gradient
    float magnitude = length(colorX) + length(colorY);

    // Use a threshold to highlight edges
    float threshold = 0.3; // Adjust this threshold as needed
    vec3 edgeColor = mix(vec3(0.0), vec3(1.0), step(threshold, magnitude));

    // Keep the original color if it's not an edge
    vec3 originalColor = texture(u_screen_texture, v_uv).rgb;
    vec3 resultColor = mix(originalColor, edgeColor, step(threshold, magnitude));

    if(resultColor == vec3(1.0f, 1.0f, 1.0f))
    {
        resultColor = vec3(0.0f);
    }

    out_colour = vec4(resultColor, 1.0); // Assign to out_colour
}
*/
