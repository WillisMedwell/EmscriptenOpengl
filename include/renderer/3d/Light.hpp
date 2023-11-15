#pragma once

#include <glaze/glaze.hpp>
#include <glm/glm.hpp>
#include <optional>

struct LightAttenuation {
    float constant = 0;
    float linear = 0;
    float quadratic = 0;
};

struct PointLight {
    glm::vec3 position;
    LightAttenuation attenuation;
    glm::u8vec3 colour;
    float intensity;
    std::optional<float> ambient_coefficient;
    std::optional<float> specular_exponent;
};

struct DirectionalLight {
    PointLight point_light;
    glm::vec3 direction;
};

template <>
struct glz::meta<LightAttenuation> {
    using T = LightAttenuation;
    static constexpr auto value = object(
        "Constant", &T::constant,
        "Linear", &T::linear,
        "Quadratic", &T::quadratic);
};

template <>
struct glz::meta<PointLight> {
    using T = PointLight;
    static constexpr auto value = object(
        "Position", &T::position,
        "Attenuation", &T::attenuation,
        "Colour", &T::colour,
        "Intensity", &T::intensity,
        "Ambient-Coefficient", &T::ambient_coefficient,
        "Specular-Exponent", &T::specular_exponent);
};

template <>
struct glz::meta<DirectionalLight> {
    using T = DirectionalLight;
    constexpr static auto value = object(
        "Properties", &T::point_light,
        "Direction", &T::direction);
};