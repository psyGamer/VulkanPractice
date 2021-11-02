#pragma once

#include <string>
#include <stb_image.h>

class Image {

private:
	bool m_Loaded;
	int m_Width, m_Height, m_Channels;
	stbi_uc* m_pPixels;

public:
	Image();
	Image(const std::string& filePath);

	~Image();

	int GetHeight();
	int GetWidth();
	int GetChannels();
	int GetSizeInBytes();

	stbi_uc* GetRawPixels();

	void LoadImage(const std::string& filePath);
	void Destory();
};