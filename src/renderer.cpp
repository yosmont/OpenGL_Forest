#ifndef STB_DEFINE_FLAGS
#define STB_IMAGE_IMPLEMENTATION
#endif // !STB_DEFINE_FLAGS

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
#include <thread>
#include <future>

#include "stb_image.hpp"

BEGIN_VISUALIZER_NAMESPACE

float skyboxVertices[] =
{
    //   Coordinates
    -1.0f, -1.0f,  1.0f,//        7--------6
     1.0f, -1.0f,  1.0f,//       /|       /|
     1.0f, -1.0f, -1.0f,//      4--------5 |
    -1.0f, -1.0f, -1.0f,//      | |      | |
    -1.0f,  1.0f,  1.0f,//      | 3------|-2
     1.0f,  1.0f,  1.0f,//      |/       |/
     1.0f,  1.0f, -1.0f,//      0--------1
    -1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
    // Right
    1, 2, 6,
    6, 5, 1,
    // Left
    0, 4, 7,
    7, 3, 0,
    // Top
    4, 5, 6,
    6, 7, 4,
    // Bottom
    0, 3, 2,
    2, 1, 0,
    // Back
    0, 1, 5,
    5, 4, 0,
    // Front
    3, 7, 6,
    6, 2, 3
};

STBIImgInfo LoadImg(std::string dirpath, std::string filename)
{
    STBIImgInfo info;
    info.data = stbi_load((dirpath + filename).c_str(), &(info.width), &(info.height), &(info.nrChannels), 0);
    if (info.data) {
        stbi_set_flip_vertically_on_load(false);
        std::cout << "texture loaded: " << filename << std::endl;
        std::cout << filename << " size: " << info.width << " * " << info.height << std::endl;
    }
    else
        std::cout << "Failed to load texture: " << filename << std::endl;
    return info;
}

void LoadDesert(std::vector<int> *indices, std::vector<VertexDataPosition3fColor3f> *vertices)
{
    //load obj:
    tinyobj::ObjReader desert = LoadObjFile("../../res/desert.obj");
    std::cout << "desert load" << std::endl;
    std::cout << "desert shape nb: " << desert.GetShapes().size() << std::endl;
    std::cout << "desert number of vertices: " << desert.GetAttrib().GetVertices().size() << std::endl;
    std::cout << "desert number of indices: " << desert.GetShapes()[0].mesh.indices.size() << std::endl;
    std::cout << "desert number of colors: " << desert.GetAttrib().colors.size() << std::endl;
    std::cout << "desert number of normals: " << desert.GetAttrib().normals.size() << std::endl;
    std::cout << "desert number of texcoords: " << desert.GetAttrib().texcoords.size() << std::endl;
    //init buffer:
    for (std::vector<tinyobj::index_t>::const_iterator i = desert.GetShapes()[0].mesh.indices.begin(); i != desert.GetShapes()[0].mesh.indices.end(); ++i)
        indices->push_back(i->vertex_index);
    for (std::vector<tinyobj::real_t>::const_iterator v = desert.GetAttrib().GetVertices().begin(); v != desert.GetAttrib().GetVertices().end(); v += 3)
        vertices->push_back(VertexDataPosition3fColor3f {
            glm::vec3 {
                *v,
                *(v + 1),
                *(v + 2)
            }, glm::vec3 {
                0.31,
                0.34,
                0.04
            }
        });
}

void LoadPalm(std::vector<int> *indices, std::vector<VertexDataPosition3fColor3f> *vertices)
{
    //load obj:
    tinyobj::ObjReader palm = LoadObjFile("../../res/palm.obj");
    int palmVSize = palm.GetAttrib().GetVertices().size();
    std::cout << "palm load" << std::endl;
    std::cout << "palm number of shape: " << palm.GetShapes().size() << std::endl;
    std::cout << "palm number of vertices: " << palmVSize << std::endl;
    std::cout << "palm number of indices: " << palm.GetShapes()[0].mesh.indices.size() << std::endl;
    std::cout << "palm number of colors: " << palm.GetAttrib().colors.size() << std::endl;
    std::cout << "palm number of normals: " << palm.GetAttrib().normals.size() << std::endl;
    std::cout << "palm number of texcoords: " << palm.GetAttrib().texcoords.size() << std::endl;
    //init buffer:
    for (std::vector<tinyobj::index_t>::const_iterator i = palm.GetShapes()[0].mesh.indices.begin(); i != palm.GetShapes()[0].mesh.indices.end(); ++i)
        indices->push_back(i->vertex_index);
    for (std::vector<tinyobj::real_t>::const_iterator v = palm.GetAttrib().GetVertices().begin(); v != palm.GetAttrib().GetVertices().end(); v += 3)
        vertices->push_back(VertexDataPosition3fColor3f{
            glm::vec3 {
                (*v),
                (*(v + 1)),
                (*(v + 2))
            }, glm::vec3 {
                0.24,
                0.18,
                0.01
            }
        });
}

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

void Renderer::ShaderError(GLuint ID, std::string type)
{
    GLint length = 0;

    GL_CALL(glGetShaderiv, ID, GL_INFO_LOG_LENGTH, &length);

    if (length > 1)
    {
        std::string log(length, '\0');

        GL_CALL(glGetShaderInfoLog, ID, length, nullptr, log.data());

        std::cerr << type << " shader log:\n" << log << '\n';
    }
}

void Renderer::ShaderProgramError(GLuint ID) 
{
    GLint length = 0;

    GL_CALL(glGetProgramiv, ID, GL_INFO_LOG_LENGTH, &length);

    if (length > 1)
    {
        std::string log(length, '\0');

        GL_CALL(glGetProgramInfoLog, ID, length, nullptr, log.data());

        std::cerr << "Shader program log:\n" << log << '\n';
    }
}

GLuint Renderer::InitShader(char const* const vertexSrc, char const* const fragmentSrc)
{
    GLuint vertexShader = GL_CALL(glCreateShader, GL_VERTEX_SHADER);
    GL_CALL(glShaderSource, vertexShader, 1, &vertexSrc, NULL);
    GL_CALL(glCompileShader, vertexShader);
    ShaderError(vertexShader, "Vertex");
    GLuint fragmentShader = GL_CALL(glCreateShader, GL_FRAGMENT_SHADER);
    GL_CALL(glShaderSource, fragmentShader, 1, &fragmentSrc, NULL);
    GL_CALL(glCompileShader, fragmentShader);
    ShaderError(fragmentShader, "Fragment");
    GLuint progID = GL_CALL(glCreateProgram);
    GL_CALL(glAttachShader, progID, vertexShader);
    GL_CALL(glAttachShader, progID, fragmentShader);
    GL_CALL(glLinkProgram, progID);
    ShaderProgramError(progID);
    GL_CALL(glDetachShader, progID, vertexShader);
    GL_CALL(glDetachShader, progID, fragmentShader);
    GL_CALL(glDeleteShader, vertexShader);
    GL_CALL(glDeleteShader, fragmentShader);
    return progID;
}

GLuint Renderer::InitSkyboxShader()
{
    char const* const vertexSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec4 pos = projection * view * vec4(aPos, 1.0f);
    // Having z equal w will always result in a depth of 1.0f
    gl_Position = vec4(pos.x, -pos.y, pos.w, pos.w);
    // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
    texCoords = vec3(aPos.x, -aPos.y, -aPos.z);
})";
    char const* const fragmentSource = R"(#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, texCoords);
})";

    GLuint progID = InitShader(vertexSource, fragmentSource);
    GL_CALL(glUseProgram, progID);
    GLint location = GL_CALL(glGetUniformLocation, progID, "skybox");
    GL_CALL(glUniform1i, location, 0);
    GL_CALL(glUseProgram, 0);
    return progID;
}

GLuint Renderer::InitDefaultShader()
{
    char const* const vertexSource = R"(#version 450 core

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) smooth out vec3 color;

layout(std140, binding = 0) uniform Matrix
{
    mat4 modelViewProjection;
};
uniform vec3 transfoModif;

void main()
{
    color = inColor;
    //gl_Position = modelViewProjection*vec4(inWorldPos, 1.);
    gl_Position = modelViewProjection*vec4(inWorldPos.x + transfoModif.x, inWorldPos.y + transfoModif.y, inWorldPos.z + transfoModif.z, 1.);
})";
    char const* const fragmentSource = R"(#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) smooth in vec3 color;

void main()
{
    outColor = vec4(color, 1.0);
})";
    return InitShader(vertexSource, fragmentSource);
}

bool Renderer::Initialize()
{
    /*constexpr uint16_t sphereStackCount = 63;
    constexpr uint16_t sphereSectorCount = 63;

    constexpr uint16_t vertexCount = (sphereStackCount + 1) * (sphereSectorCount + 1);
    constexpr uint16_t indexCount = (sphereStackCount - 1) * sphereSectorCount * 6;

    m_IndexCount = indexCount;

    std::vector<VertexDataPosition3fColor3f> oldVertices(vertexCount);
    std::vector<uint16_t> oldIndices(indexCount);

    GenerateSphereMesh(oldVertices, oldIndices, sphereStackCount, sphereSectorCount, glm::vec3(0.0f), 1.0f);*/

    
    
    //adding the desert to the buffer
    std::vector<VertexDataPosition3fColor3f> vertices[2] = { std::vector<VertexDataPosition3fColor3f>(), std::vector<VertexDataPosition3fColor3f>()};
    std::vector<int> indices[2] = { std::vector<int>(), std::vector<int>() };
    std::future<void> loader[2];
    loader[0] = std::async(LoadDesert, &(indices[0]), &(vertices[0]));
    //adding all the palm to the buffer
    loader[1] = std::async(LoadPalm, &(indices[1]), &(vertices[1]));
    std::future<std::vector<glm::vec4>> loaderTransfo = std::async(LoadTransfoFile, "../../res/palmTransfo.txt");
    std::string skyboxDir = "../../res/DesertSkybox/";
    std::string facesCubemap[6] = {
        "Right.png",
        "Left.png",
        "Up.png",
        "Down.png",
        "Front.png",
        "Back.png"
    };
    std::future<STBIImgInfo> loaderTexture[6];
    for (unsigned int i = 0; i < 6; ++i) {
        loaderTexture[i] = std::async(LoadImg, skyboxDir, facesCubemap[i]);
    }
    m_TransfoPalm = loaderTransfo.get();
    std::cout << "transfoPalm size: " << m_TransfoPalm.size() << std::endl;
    loader[0].wait();
    loader[1].wait();

    GL_CALL(glCreateBuffers, 1, &m_UBO);
    GL_CALL(glNamedBufferStorage, m_UBO, sizeof(glm::mat4), glm::value_ptr(m_Camera->GetViewProjectionMatrix()), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

    GL_CALL(glCreateBuffers, 3, m_IBO);
    GL_CALL(glCreateBuffers, 3, m_VBO);
    for (int i = 0; i < 2; ++i) {
        std::cout << "indices[" << i << "] size: " << indices[i].size() << std::endl;
        std::cout << "vertices[" << i << "] size: " << vertices[i].size() << std::endl;
        m_IndexCount[i] = indices[i].size();
        GL_CALL(glNamedBufferStorage, m_IBO[i], sizeof(int) * indices[i].size(), indices[i].data(), 0);
        GL_CALL(glNamedBufferStorage, m_VBO[i], sizeof(VertexDataPosition3fColor3f) * vertices[i].size(), vertices[i].data(), 0);
    }

    GL_CALL(glCreateVertexArrays, 3, m_VAO);
    for (int i = 0; i < 2; ++i) {
        GL_CALL(glBindVertexArray, m_VAO[i]);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO[i]);
        GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_IBO[i]);

        GL_CALL(glEnableVertexAttribArray, 0);
        GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataPosition3fColor3f), nullptr);
        GL_CALL(glEnableVertexAttribArray, 1);
        GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexDataPosition3fColor3f), reinterpret_cast<GLvoid*>(sizeof(glm::vec3)));

        GL_CALL(glBindVertexArray, 0);

        GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
        GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);

        GL_CALL(glDisableVertexAttribArray, 0);
        GL_CALL(glDisableVertexAttribArray, 1);
    }

    m_ShaderProgram[0] = InitDefaultShader();

    m_UBOData = GL_CALL_REINTERPRET_CAST_RETURN_VALUE(glm::mat4*, glMapNamedBufferRange, m_UBO, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

    m_ShaderProgram[1] = InitSkyboxShader();
    GL_CALL(glBindVertexArray, m_VAO[2]);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, m_VBO[2]);
    GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, m_IBO[2]);
    GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    GL_CALL(glEnableVertexAttribArray, 0);
    GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    GL_CALL(glBindVertexArray, 0);
    GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
    GL_CALL(glGenTextures, 1, &m_Texture);
    GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, m_Texture);
    GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // These are very important to prevent seams
    GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CALL(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL_CALL(glEnable, GL_TEXTURE_CUBE_MAP_SEAMLESS);
    for (unsigned int i = 0; i < 6; ++i) {
        STBIImgInfo info = loaderTexture[i].get();
        if (info.data) {
            GL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, info.width, info.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info.data);
        }
        stbi_image_free(info.data);
    }

    return true;
}

void Renderer::Render()
{
    GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GL_CALL(glUseProgram, m_ShaderProgram[0]);

    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, m_UBO);

    glm::vec4 tmp(0, 0, 0, 0);
    GLint transfoModifLocation = GL_CALL(glGetUniformLocation, m_ShaderProgram[0], "transfoModif");
    GL_CALL(glUniform3f, transfoModifLocation, 0, 0, 0);

    GL_CALL(glBindVertexArray, m_VAO[0]);
    GL_CALL(glDrawElements, GL_TRIANGLES, m_IndexCount[0], GL_UNSIGNED_INT, nullptr);
    GL_CALL(glBindVertexArray, 0);
    for (int i = 0; i < m_TransfoPalm.size(); ++i) {
        GLint transfoModifLocation = GL_CALL(glGetUniformLocation, m_ShaderProgram[0], "transfoModif");
        GL_CALL(glUniform3f, transfoModifLocation, m_TransfoPalm[i].x, m_TransfoPalm[i].y, m_TransfoPalm[i].z);

        GL_CALL(glBindVertexArray, m_VAO[1]);
        GL_CALL(glDrawElements, GL_TRIANGLES, m_IndexCount[1], GL_UNSIGNED_INT, nullptr);
        GL_CALL(glBindVertexArray, 0);
    }
    GL_CALL(glBindBufferBase, GL_UNIFORM_BUFFER, 0, 0);

    GL_CALL(glUseProgram, 0);

    GL_CALL(glDepthFunc, GL_LEQUAL);
    GL_CALL(glUseProgram, m_ShaderProgram[1]);
    
    m_SkyboxInfo.view = glm::mat4(1.0f);
    m_SkyboxInfo.projection = glm::mat4(1.0f);
    m_SkyboxInfo.view = glm::mat4(glm::mat3(glm::lookAt(m_Camera->GetPosition(), m_Camera->GetPosition() - m_Camera->GetDirection(), m_Camera->GetUp())));
    m_SkyboxInfo.projection = glm::perspective(glm::radians(45.0f), (float)m_ViewportWidth / m_ViewportHeight, 0.1f, 100.0f);
    GLint viewLocation = GL_CALL(glGetUniformLocation, m_ShaderProgram[1], "view");
    GLint projectionLocation = GL_CALL(glGetUniformLocation, m_ShaderProgram[1], "projection");
    GL_CALL(glUniformMatrix4fv, viewLocation, 1, GL_FALSE, glm::value_ptr(m_SkyboxInfo.view));
    GL_CALL(glUniformMatrix4fv, projectionLocation, 1, GL_FALSE, glm::value_ptr(m_SkyboxInfo.projection));

    GL_CALL(glBindVertexArray, m_VAO[2]);
    GL_CALL(glActiveTexture, GL_TEXTURE0);
    GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, m_Texture);
    GL_CALL(glDrawElements, GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    GL_CALL(glBindVertexArray, 0);
    GL_CALL(glUseProgram, 0);
    GL_CALL(glDepthFunc, GL_LESS);

}

void Renderer::Cleanup()
{
    m_UBOData = nullptr;

    GL_CALL(glUnmapNamedBuffer, m_UBO);

    GL_CALL(glDeleteBuffers, 3, m_VBO);
    GL_CALL(glDeleteBuffers, 3, m_IBO);
    GL_CALL(glDeleteBuffers, 1, &m_UBO);

    GL_CALL(glDeleteVertexArrays, 3, m_VAO);

    GL_CALL(glDeleteProgram, m_ShaderProgram[0]);
    GL_CALL(glDeleteProgram, m_ShaderProgram[1]);

    GL_CALL(glDeleteTextures, 1, &m_Texture);
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
