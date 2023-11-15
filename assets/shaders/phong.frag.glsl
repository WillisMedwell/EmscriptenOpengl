#version 300
precision highp float;
out vec4 out_colour;


in vec4 v_pos;
in vec4 v_normal;

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

vec3 ComputePhongLighting(PointLight light, vec3 position, vec3 normal) {
    vec3 lightDir = normalize(light.position - position);

    // Ambient
    vec3 ambient = light.intensity * light.ambient_coefficient * light.colour;

    // Diffuse
    float diff = smoothstep(0.0, 1.0, dot(normal, lightDir));
    vec3 diffuse = light.intensity * diff * light.colour;

    // Specular
    vec3 viewDir = normalize(-position);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), light.specular_exponent);
    vec3 specular = light.intensity * spec * light.colour;

    // Attenuation
    float distance = length(light.position - position);
    float attenuation = 1.0 / (light.constant + light.linear * sqrt(distance) + light.quadratic * (distance));

    return (ambient + (diffuse /*+ specular*/) * attenuation);
}

void main() {
    vec3 finalColour = vec3(0.0, 0.0, 0.0);
    vec3 normal = vec3(normalize(v_normal));

    for(int i = 0; i < int(round(u_point_lights_size)); ++i) {
        finalColour += ComputePhongLighting(u_point_lights[i], v_pos.xyz, normal.xyz);
    }

    // Apply gamma correction
    finalColour = pow(finalColour, vec3(1.0 / 2.2));
    finalColour = clamp(finalColour, 0.0, 1.0);  // Clamping final color

    out_colour = vec4(finalColour, 1.0);
}
