#ifndef RENDERER_HPP
#define RENDERER_HPP

BEGIN_VISUALIZER_NAMESPACE

class Camera;

struct VertexDataPosition3fColor3f
{
    glm::vec3 position;
    glm::vec3 color;
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
    /*void LoadDesert(std::vector<int>& indices, std::vector<VertexDataPosition3fColor3f>& vertices);
    void LoadPalm(std::vector<int>& indices, std::vector<VertexDataPosition3fColor3f>& vertices);*/

    GLuint m_UBO, m_VBO[2], m_IBO[2], m_VAO[2], m_ShaderProgram;

    uint32_t m_IndexCount[2];

    glm::mat4* m_UBOData;

    std::shared_ptr<Camera> m_Camera;
    uint32_t m_ViewportWidth, m_ViewportHeight;
};

END_VISUALIZER_NAMESPACE

#endif // !RENDERER_HPP
