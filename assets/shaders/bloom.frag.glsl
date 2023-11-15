#version 300

in vec2 v_uv;

uniform sampler2D u_screen_texture;

void main() {
    // guide via https://github.com/lettier/3d-game-shaders-for-beginners/blob/master/demonstration/shaders/fragment/bloom.frag

    int size = 4;
    float separation = 1.75;
    float threshold = 0.8;
    float amount = 0.5;

    vec2 texture_size = textureSize(u_screen_texture, 0).xy;
    vec4 result = vec4(0.0);
    vec4 colour = vec4(0.0);
    float value = 0.0;
    float count = 0.0;

    for(int i = -size; i <= size; ++i) {
        for(int j = -size; j <= size; ++j) {

            colour = texture(u_screen_texture, (vec2(i, j) * separation + gl_FragCoord.xy) / texture_size);

            value = max(colour.r, max(colour.g, colour.b));
            if(value < threshold) {
                colour = vec4(0.0);
            }

            result += colour;
            count += 1.0;
        }
    }
    result /= count;

    colour = mix(vec4(0.0), result, amount);

    vec4 t = texture(u_screen_texture, v_uv);

    colour.r = max(t.r, colour.r);
    colour.g = max(t.g, colour.g);
    colour.b = max(t.b, colour.b);
    colour.a = max(t.a, colour.a);

    gl_FragColor = colour;
}
