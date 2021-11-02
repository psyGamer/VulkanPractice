#include "Image.h"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image::Image() {
	m_Loaded = false;

	m_Width = 0;
	m_Height = 0;
	m_Channels = 0;

	m_pPixels = nullptr;
}

Image::Image(const std::string& filePath) {
	LoadImage(filePath);
}

Image::~Image() {
	Destory();
}

int Image::GetHeight() {
	if (!m_Loaded) {
		std::cerr << "This image isn't loaded!";
		throw std::logic_error("This image isn't loaded!");
	}

	return m_Height;
}
int Image::GetWidth() {
	if (!m_Loaded) {
		std::cerr << "This image isn't loaded!";
		throw std::logic_error("This image isn't loaded!");
	}

	return m_Width;
}
int Image::GetChannels() {
	if (!m_Loaded) {
		std::cerr << "This image isn't loaded!";
		throw std::logic_error("This image isn't loaded!");
	}

	return 4;
}
int Image::GetSizeInBytes() {
	if (!m_Loaded) {
		std::cerr << "This image isn't loaded!";
		throw std::logic_error("This image isn't loaded!");
	}

	return GetWidth() * GetHeight() * GetChannels();
}

stbi_uc* Image::GetRawPixels() {
	if (!m_Loaded) {
		std::cerr << "This image isn't loaded!";
		throw std::logic_error("This image isn't loaded!");
	}

	return m_pPixels;
}

void Image::LoadImage(const std::string& filePath) {
	if (m_Loaded) {
		std::cerr << "This image is already loaded!";
		throw std::logic_error("This image is already loaded!");
	}

	m_pPixels = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);

	if (m_pPixels == nullptr) {
		std::cerr << "Could not load image: " + filePath;
		throw std::invalid_argument("Could not load image: " + filePath);
	}

	m_Loaded = true;
}

void Image::Destory() {
	if (!m_Loaded)
		return;

	stbi_image_free(m_pPixels);

	m_Loaded = false;
}
