#include <GL/glew.h>

BEGIN_VISUALIZER_NAMESPACE

void CheckGLError(const std::string_view& functionName, const std::string_view& fileName, const int32_t lineNumber)
{
    GLenum err = glGetError();

    while (err != GL_NO_ERROR)
    {
        std::string_view error = "Unknown error";

        switch (err)
        {
        case GL_INVALID_ENUM:
            error = "invalid enum";
            break;
        case GL_INVALID_VALUE:
            error = "invalid value";
            break;
        case GL_INVALID_OPERATION:
            error = "invalid operation";
            break;
        case GL_OUT_OF_MEMORY:
            error = "out of memory";
            break;
        case GL_STACK_UNDERFLOW:
            error = "stack underflow";
            break;
        case GL_STACK_OVERFLOW:
            error = "stack overflow";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "invalid framebuffer operation";
            break;
        }

        std::cerr << "Error in file " << fileName << " line " << lineNumber << " when calling " << functionName << ": " << error << '\n';
        err = glGetError();
    }
}

END_VISUALIZER_NAMESPACE
