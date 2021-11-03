#include "FileUtil.h"

#include <iostream>
#include <fstream>

namespace FileUtil {

	std::vector<char> ReadFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);

		if (file) {
			size_t fileSize = (size_t)file.tellg();
			std::vector<char> fileBuffer(fileSize);

			file.seekg(0);
			file.read(fileBuffer.data(), fileSize);
			file.close();

			return fileBuffer;
		}

		std::cerr << "Failed to open file: " << filePath;
		throw std::runtime_error("Failed to open file!");
	}
}