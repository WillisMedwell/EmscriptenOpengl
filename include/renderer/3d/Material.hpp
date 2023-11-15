#pragma once

#include <glm/glm.hpp>

struct Material
{
    float specular_exponent;
    glm::vec3 ambient_colour;
    glm::vec3 diffuse_colour;
    glm::vec3 specular_colour;
    glm::vec3 emissive_colour;
    float optical_density;
    float dissolve_factor;
};

namespace MaterialLoader
{
    auto fromMtl()
}