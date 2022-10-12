#ifndef GLUTILS_HPP
#define GLUTILS_HPP

BEGIN_VISUALIZER_NAMESPACE

void CheckGLError(const std::string_view& functionName, const std::string_view& fileName, const int32_t lineNumber);

// Define this to 0 if you don't want to check errors anymore
#define DEBUG_GL_CALLS 1

#if DEBUG_GL_CALLS
#define GL_CALL(x, ...)    x(__VA_ARGS__); CheckGLError(#x, __FILE__, __LINE__)
#define GL_CALL_REINTERPRET_CAST_RETURN_VALUE(type, x, ...)    reinterpret_cast<type>(x(__VA_ARGS__)); CheckGLError(#x, __FILE__, __LINE__)
#else
#define GL_CALL(x, ...)    x(__VA_ARGS__);
#define GL_CALL_REINTERPRET_CAST_RETURN_VALUE(type, x, ...)    reinterpret_cast<type>(x(__VA_ARGS__));
#endif

END_VISUALIZER_NAMESPACE

#endif // !GLUTILS_HPP
