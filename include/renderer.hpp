#ifndef RENDERER_HPP
#define RENDERER_HPP

BEGIN_VISUALIZER_NAMESPACE

class Camera;

struct VertexDataPosition3fColor3f
{
    glm::vec3 position;
    glm::vec3 color;
};

struct SkyboxInfo
{
    glm::mat4 view;
    glm::mat4 projection;
};

class Renderer
{
public:
    Renderer(uint32_t width, uint32_t height, const std::shared_ptr<Camera>& camera)
        : m_ViewportWidth(width)
        , m_ViewportHeight(height)
        , m_Camera(camera)
    {}

    Renderer() = delete;
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool Initialize();
    void Render();
    void Cleanup();

    void UpdateViewport(uint32_t width, uint32_t height);
    void UpdateCamera();

private:
    void ShaderError(GLuint ID, std::string type);
    void ShaderProgramError(GLuint ID);
    GLuint InitShader(char const* const vertexSrc, char const* const fragmentSrc);
    GLuint InitSkyboxShader();
    GLuint InitDefaultShader();

    GLuint m_UBO, m_VBO[3], m_IBO[3], m_VAO[3], m_ShaderProgram[2], m_Texture;

    uint32_t m_IndexCount[2];

    glm::mat4* m_UBOData;

    SkyboxInfo m_SkyboxInfo;

    std::vector<glm::vec4> m_TransfoPalm;

    std::shared_ptr<Camera> m_Camera;
    uint32_t m_ViewportWidth, m_ViewportHeight;
};

END_VISUALIZER_NAMESPACE

#endif // !RENDERER_HPP
