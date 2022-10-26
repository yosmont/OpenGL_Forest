#ifndef UTILS_HPP
#define UTILS_HPP

#include <tiny_obj_loader.hpp>
#include <glm/glm.hpp>

BEGIN_VISUALIZER_NAMESPACE

bool LoadFile(const std::string& fileName, std::string& result);
void DisplayLastWinAPIError();
tinyobj::ObjReader LoadObjFile(std::string inputFile, tinyobj::ObjReaderConfig readerConfig = tinyobj::ObjReaderConfig());
std::vector<glm::vec4> LoadTransfoFile(std::string inputFile);

END_VISUALIZER_NAMESPACE

#endif // !UTILS_HPP
