#include <GL/glew.h>

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop, 0)

#include <utils.hpp>
#include <glutils.hpp>
#include <camera.hpp>
#include <renderer.hpp>

BEGIN_VISUALIZER_NAMESPACE

struct VertexDataPosition3fColor3f
{
    glm::vec3 position;
    glm::vec3 color;
};

void GenerateSphereMesh(std::vector<VertexDataPosition3fColor3f>& vertices, std::vector<uint16_t>& indices, uint16_t sphereStackCount, uint16_t sphereSectorCount, glm::vec3 sphereCenter, float sphereRadius)
{
    std::size_t vertexId = 0;

    for (uint16_t i = 0; i <= sphereStackCount; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(sphereStackCount);

        const float stackAngle = glm::pi<float>() / 2.0f - t * glm::pi<float>();

        const float nxz = glm::cos(stackAngle);
        const float ny = glm::sin(stackAngle);

        const float xz = sphereRadius * nxz;
        const float y = sphereRadius * ny;

        const float mul = stackAngle < 0.0f ? -1.0f : 1.0f;

        const float dxz = -ny * mul;

        // When stackAngle is null dxz is equal to 0 and it is impossible to compute the tangents and bitangents
        // So we use the up vector as a bitangent
        const bool useUpVector = (sphereStackCount % 2 == 0) && (i == sphereStackCount / 2);

        for (uint16_t j = 0; j <= sphereSectorCount; ++j)
        {
            const float s = static_cast<float>(j) / static_cast<float>(sphereSectorCount);

            const float sectorAngle = s * 2.0f * glm::pi<float>();

            const float csa = glm::sin(sectorAngle);
            const float ssa = glm::cos(sectorAngle);

            const float nx = nxz * csa;
            const float nz = nxz * ssa;

            const float x = xz * csa;
            const float z = xz * ssa;

            const glm::vec3 direction = glm::vec3(x, y, z);

            const glm::vec3 position = sphereCenter + direction;

            const glm::vec2 texcoords = glm::vec2(s, t);

            const glm::vec3 normal = glm::vec3(nx, ny, nz);

            const glm::vec3 color = glm::vec3(0.5f) + normal * 0.5f;

            vertices[vertexId++] = { position, color };

            if (useUpVector)
            {
                const glm::vec3 bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
                const glm::vec3 tangent = glm::cross(normal, bitangent);
            }
            else
            {
                const glm::vec3 tangent = glm::normalize(glm::vec3(dxz * -glm::sin(sectorAngle), dxz * glm::cos(sectorAngle), 0.0f));
                const glm::vec3 bitangent = glm::cross(tangent, normal);
            }
        }
    }

    const uint16_t sectorCountplusOne = sphereSectorCount + 1;

    std::size_t indexID = 0;

    for (uint16_t j = 0; j < sphereSectorCount; ++j)
    {
        const uint16_t jp1 = j + 1;

        indices[indexID++] = j;
        indices[indexID++] = sectorCountplusOne + j;
        indices[indexID++] = sectorCountplusOne + jp1;
    }

    for (uint16_t i = 1; i < sphereStackCount - 1; ++i)
    {
        const uint16_t k1 = i * sectorCountplusOne;
        const uint16_t k2 = k1 + sectorCountplusOne;

        for (uint16_t j = 0; j < sphereSectorCount; ++j)
        {
            const uint16_t jp1 = j + 1;

            indices[indexID++] = k1 + j;
            indices[indexID++] = k2 + j;
            indices[indexID++] = k2 + jp1;
            
            indices[indexID++] = k1 + j;
            indices[indexID++] = k2 + jp1;
            indices[indexID++] = k1 + jp1;            
        }
    }

    const uint16_t k1 = (sphereStackCount - 1) * sectorCountplusOne;
    const uint16_t k2 = k1 + sectorCountplusOne;

    for (uint16_t j = 0; j < sphereSectorCount; ++j)
    {
        const uint16_t jp1 = j + 1;

        indices[indexID++] = k1 + j;
        indices[indexID++] = k2 + j;        
        indices[indexID++] = k1 + jp1;
    }
}

bool Renderer::Initialize()
{
    constexpr uint16_t sphereStackCount = 63;
    constexpr uint16_t sphereSectorCount = 63;

    constexpr uint16_t vertexCount = (sphereStackCount + 1) * (sphereSectorCount + 1);
    constexpr uint16_t indexCount = (sphereStackCount - 1) * sphereSectorCount * 6;

    m_IndexCount = indexCount;

    std::vector<VertexDataPosition3fColor3f> vertices(vertexCount);
    std::vector<uint16_t> indices(indexCount);

    GenerateSphereMesh(vertices, indices, sphereStackCount, sphereSectorCount, glm::vec3(0.0f), 1.0f);

    //tinyobj::ObjReader palm = LoadObjFile("../../res/desert.obj");
    /*std::cout << "desert load" << std::endl;
    std::cout << "desert shape nb: " << desert.GetShapes().size() << std::endl;
    std::cout << "desert number of vertices: " << desert.GetAttrib().GetVertices().size() << std::endl;
    std::cout << "desert number of indices: " << desert.GetShapes()[0].mesh.indices.size() << std::endl;*/
    tinyobj::ObjReader palm = LoadObjFile("../../res/palm.obj");
    std::cout << "palm load" << std::endl;
    std::cout << "palm number of shape: " << palm.GetShapes().size() << std::endl;
    std::cout << "palm number of vertices: " << palm.GetAttrib().GetVertices().size() << std::endl;
    std::cout << "palm number of indices: " << palm.GetShapes()[0].mesh.indices.size() << std::endl;
    std::vector<int> palmVIndices;
    for (auto i = palm.GetShapes()[0].mesh.indices.begin(); i != palm.GetShapes()[0].mesh.indices.end(); ++i) {
        //std::cout << i->vertex_index << std::endl;
        palmVIndices.push_back(i->vertex_index);
    }
    std::cout << "palmVIndices size: " << palmVIndices.size() << std::endl;

    GL_CALL(glCreateBuffers, 1, &m_UBO);
    GL_CALL(glNamedBufferStorage, m_UBO, sizeof(glm::mat4), glm::value_ptr(m_Camera->GetViewProjectionMatrix()), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    GL_CALL(glCreateBuffers, 1, &m_VBO);
    //GL_CALL(glNamedBufferStorage, m_VBO, sizeof(VertexDataPosition3fColor3f) * vertexCount, vertices.data(), 0);
    GL_CALL(glNamedBufferStorage, m_VBO, sizeof(tinyobj::real_t) * palm.GetAttrib().GetVertices().size(), palm.GetAttrib().GetVertices().data(), 0);

    GL_CALL(glCreateBuffers, 1, &m_IBO);
    //GL_CALL(glNamedBufferStorage, m_IBO, sizeof(uint16_t) * indexCount, indices.data(), 0);
    GL_CALL(glNamedBufferStorage, m_IBO, sizeof(int) * palmVIndices.size(), palmVIndices.data(), 0);

    GL_CALL(glCreateVertexArrays, 1, &m_VAO);
    GL_CALL(glBindVertexArray, m_VAO);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO);
    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_IBO);

    /*GL_CALL(glEnableVertexAttribArray, 0);
    GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataPosition3fColor3f), nullptr);
    GL_CALL(glEnableVertexAttribArray, 1);
    GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataPosition3fColor3f), reinterpret_cast<GLvoid*>(sizeof(glm::vec3)));*/
    GL_CALL(glEnableVertexAttribArray, 0);
    GL_CALL(glVertexAttribPointer, 0, 1, GL_FLOAT, GL_FALSE, sizeof(tinyobj::real_t) * 3, nullptr);
    GL_CALL(glEnableVertexAttribArray, 1);
    GL_CALL(glVertexAttribPointer, 1, 1, GL_FLOAT, GL_FALSE, sizeof(tinyobj::real_t) * 3, reinterpret_cast<GLvoid*>(sizeof(tinyobj::real_t)));
    GL_CALL(glEnableVertexAttribArray, 2);
    GL_CALL(glVertexAttribPointer, 2, 1, GL_FLOAT, GL_FALSE, sizeof(tinyobj::real_t) * 3, reinterpret_cast<GLvoid*>(sizeof(tinyobj::real_t) * 2));

    GL_CALL(glBindVertexArray, 0);

    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);

    GL_CALL(glDisableVertexAttribArray, 0);
    GL_CALL(glDisableVertexAttribArray, 1);

    GLuint vShader = GL_CALL(glCreateShader, GL_VERTEX_SHADER);
    GLuint fShader = GL_CALL(glCreateShader, GL_FRAGMENT_SHADER);

    m_ShaderProgram = glCreateProgram();

    GL_CALL(glAttachShader, m_ShaderProgram, vShader);
    GL_CALL(glAttachShader, m_ShaderProgram, fShader);

    {
        char const* const vertexShader =
R"(#version 450 core

layout(location = 0) in float posX;
layout(location = 1) in float posY;
layout(location = 2) in float posZ;

layout(location = 0) smooth out vec3 color;

layout(std140, binding = 0) uniform Matrix
{
    mat4 modelViewProjection;
};

void main()
{
    color = vec3(220, 220, 220);
    gl_Position = modelViewProjection*vec4(posX, posY, posZ, 1.);
}
)";

        GL_CALL(glShaderSource, vShader, 1, &vertexShader, nullptr);

        GL_CALL(glCompileShader, vShader);

        {
            GLint length = 0;

            GL_CALL(glGetShaderiv, vShader, GL_INFO_LOG_LENGTH, &length);

            if (length > 1)
            {
                std::string log(length, '\0');

                GL_CALL(glGetShaderInfoLog, vShader, length, nullptr, log.data());

                std::cerr << "Vertex shader log:\n" << log << '\n';
            }
        }

        char const* const fragmentShader =
R"(#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) smooth in vec3 color;

void main()
{
    outColor = vec4(color, 1.0);
}
)";

        GL_CALL(glShaderSource, fShader, 1, &fragmentShader, nullptr);

        GL_CALL(glCompileShader, fShader);

        {
            GLint length = 0;

            GL_CALL(glGetShaderiv, fShader, GL_INFO_LOG_LENGTH, &length);

            if (length > 1)
            {
                std::string log(length, '\0');

                GL_CALL(glGetShaderInfoLog, fShader, length, nullptr, log.data());

                std::cerr << "Vertex shader log:\n" << log << '\n';
            }
        }
    }

    GL_CALL(glLinkProgram, m_ShaderProgram);

    {
        GLint length = 0;

        GL_CALL(glGetProgramiv, m_ShaderProgram, GL_INFO_LOG_LENGTH, &length);

        if (length > 1)
        {
            std::string log(length, '\0');

            GL_CALL(glGetProgramInfoLog, m_ShaderProgram, length, nullptr, log.data());

            std::cerr << "Shader program log:\n" << log << '\n';
        }
    }

    GL_CALL(glDetachShader, m_ShaderProgram, vShader);
    GL_CALL(glDetachShader, m_ShaderProgram, fShader);

    GL_CALL(glDeleteShader, vShader);
    GL_CALL(glDeleteShader, fShader);

    m_UBOData = GL_CALL_REINTERPRET_CAST_RETURN_VALUE(glm::mat4*, glMapNamedBufferRange, m_UBO, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    return true;
}

void Renderer::Render()
{
    GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GL_CALL(glUseProgram, m_ShaderProgram);

    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, m_UBO);
    GL_CALL(glBindVertexArray, m_VAO);
    GL_CALL(glDrawElements, GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    GL_CALL(glBindVertexArray, 0);
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, 0);

    GL_CALL(glUseProgram, 0);
}

void Renderer::Cleanup()
{
    m_UBOData = nullptr;

    GL_CALL(glUnmapNamedBuffer, m_UBO);

    GL_CALL(glDeleteBuffers, 1, &m_VBO);
    GL_CALL(glDeleteBuffers, 1, &m_IBO);
    GL_CALL(glDeleteBuffers, 1, &m_UBO);

    GL_CALL(glDeleteVertexArrays, 1, &m_VAO);

    GL_CALL(glDeleteProgram, m_ShaderProgram);

}

void Renderer::UpdateViewport(uint32_t width, uint32_t height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    m_Camera->ComputeProjection(m_ViewportWidth, m_ViewportHeight);

    std::memcpy(m_UBOData, glm::value_ptr(m_Camera->GetViewProjectionMatrix()), sizeof(glm::mat4));
    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(glm::mat4));
}

void Renderer::UpdateCamera()
{
    std::memcpy(m_UBOData, glm::value_ptr(m_Camera->GetViewProjectionMatrix()), sizeof(glm::mat4));
    GL_CALL(glFlushMappedNamedBufferRange, m_UBO, 0, sizeof(glm::mat4));
}

END_VISUALIZER_NAMESPACE
