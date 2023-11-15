#version 300


in vec3 v_normal;
in vec3 v_frag_position;

struct PointLight {
    vec3 position;
    vec3 colour;
    float intensity;

    float constant;
    float linear;
    float quadratic;

    float ambient_coefficient;
    float specular_exponent;
};
#define MAX_POINT_LIGHTS 10

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform float u_point_lights_size;

uniform vec3 warm_colour = vec3(0.87, 0.0, 0.0);
uniform vec3 middle_colour = vec3(0.73, 0.0, 0.7);
uniform vec3 cool_colour = vec3(0.0, 0.11, 0.71);

uniform float blend_factor = 0.5;

void main() {
    vec3 normal = normalize(v_normal);

    vec3 colour = vec3(0, 0, 0);

    for(int i = 0; i < u_point_lights_size; ++i) {
        colour = u_point_lights[i].colour * u_point_lights[i].ambient_coefficient * u_point_lights[i].constant * u_point_lights[i].intensity * u_point_lights[i].constant * u_point_lights[i].quadratic * u_point_lights[i].linear * u_point_lights[i].specular_exponent;
    }

    float diffuse = -1.0;
    for(int i = 0; i < u_point_lights_size; ++i) {
        vec3 light_direction = normalize(u_point_lights[i].position - v_frag_position);
        diffuse = max(dot(normal, light_direction), diffuse);
    }
    
    if(diffuse > 0) {
        float factor = pow(diffuse, 0.8);
        colour = mix(middle_colour, cool_colour, factor);
    } else {
        diffuse = abs(diffuse);
        float factor = pow(diffuse, 0.8);
        colour = mix(middle_colour, warm_colour, factor);
    }
    gl_FragColor = vec4(mix(vec3(diffuse), colour, blend_factor), 1.0);
}