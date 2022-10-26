#ifndef TINYOBJLOADER_DEFINE_FLAGS
#define TINYOBJLOADER_DEFINE_FLAGS
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#endif // !TINYOBJLOADER_DEFINE_FLAGS


#include <utils.hpp>
#include <Windows.h>

BEGIN_VISUALIZER_NAMESPACE

bool LoadFile(const std::string& fileName, std::string& result)
{
    std::ifstream ifs(fileName, std::ios::binary);

    if (ifs)
    {
        result = std::move(std::string(std::istreambuf_iterator<char>(ifs), {}));

        if (!ifs)
        {
            std::cerr << "Error: only " << ifs.gcount() << " could be read\n";
        }
    }
    else
    {
        std::cerr << "Cannot open file : " << fileName << '\n';
        return false;
    }

    return true;
}

void DisplayLastWinAPIError()
{
    DWORD error = GetLastError();

    if (error)
    {
        LPVOID lpMsgBuf;

        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL);

        if (bufLen)
        {
            LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;

            std::cerr << lpMsgStr << '\n';

            LocalFree(lpMsgBuf);
        }
    }
}

tinyobj::ObjReader LoadObjFile(std::string inputFile, tinyobj::ObjReaderConfig readerConfig)
{
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(inputFile, readerConfig)) {
        std::cerr << "LoadObjFile: read error" << reader.Error();
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }
    return reader;
}

std::vector<glm::vec4> LoadTransfoFile(std::string inputFile)
{
    std::vector<glm::vec4> ret;
    return ret;
}

END_VISUALIZER_NAMESPACE
