#pragma once

#include <concepts>
#include <type_traits>

#include <glm/glm.hpp>

#include "Concept.hpp"

#include "renderer/core/IndexBuffer.hpp"
#include "renderer/core/Shader.hpp"
#include "renderer/core/Texture.hpp"
#include "renderer/core/VertexArray.hpp"
#include "renderer/core/VertexBuffer.hpp"
#include "renderer/core/VertexBufferLayout.hpp"

#include "renderer/3d/Mesh.hpp"
#include "renderer/3d/Light.hpp"

struct SharedIndexBuffer {
    static IndexBuffer ib;
    static std::vector<uint32_t> index_buffer_data;
    static bool hasCapacityFor(size_t num_faces);
    static void makeCapacityFor(size_t num_faces);
};

template <MeshType mesh_type>
class MeshRenderer {
};

template <>
class MeshRenderer<MeshType::positions_normals_uvs> {
    constexpr static auto getLayout() -> VertexBufferLayout
    {
        VertexBufferLayout vbl;
        vbl.push<float>(3); // positions
        vbl.push<float>(3); // normals
        vbl.push<float>(2); // uvs

        return vbl;
    }

    VertexBuffer m_vb;
    VertexArray m_va;

    void bindAll();
    void unbindAll();

public:
    void init();
    void stop();
    
    void draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const Mesh<MeshType::positions_normals_uvs>& mesh, Shader& shader);
    void draw(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const Mesh<MeshType::positions_normals_uvs>& mesh, Shader& shader, const std::vector<PointLight>& lights);
};