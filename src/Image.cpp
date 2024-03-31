#include "Image.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Image::Image(const std::string& filename) {
  int width_s = 0;
  int height_s = 0;

  unsigned char* data = stbi_load(filename.c_str(), &width_s, &height_s, nullptr, 3);

  if (!data) {
    throw std::runtime_error("Could not load provided image file: " + filename + "\n");
  }

  width_ = static_cast<unsigned int>(width_s);
  height_ = static_cast<unsigned int>(height_s);
  len_ = 3 * width_ * height_;
  pixels_ = std::make_unique<float[]>(len_);

  for (unsigned int i = 0; i < len_; ++i) {
    pixels_[i] = Color::sRGBDecode(reinterpret_cast<unsigned char*>(data)[i] / 255.0f);
  }

  stbi_image_free(data);
}

Image::Image(unsigned int width, unsigned int height) : width_(width), height_(height) {
  len_ = 3 * width * height;
  pixels_ = std::make_unique<float[]>(len_);
  memset(pixels_.get(), 0, len_ * sizeof(float));
}

Image::Image(const Image& other) {
  width_ = other.width_;
  height_ = other.height_;
  len_ = other.len_;

  pixels_ = std::make_unique<float[]>(len_);
  memcpy(pixels_.get(), other.pixels_.get(), len_ * sizeof(float));
}

Image::Image(Image&& other) {
  width_ = other.width_;
  height_ = other.height_;
  len_ = other.len_;
  pixels_ = std::move(other.pixels_);
}

void Image::clear(const Color& color) {
  for (unsigned int i = 0; i < len_;) {
    pixels_[i++] = color.r;
    pixels_[i++] = color.g;
    pixels_[i++] = color.b;
  }
}

void Image::drawRectangle(const Color& color, unsigned int x, unsigned int y, unsigned int width,
                          unsigned int height) {
  for (unsigned int ry = 0; ry < height; ++ry) {
    for (unsigned int rx = 0; rx < width; ++rx) {
      const auto targetX = x + rx;
      const auto targetY = y + ry;

      setPixel(targetX, targetY, color);
    }
  }
}

void Image::drawImage(const Image& image, unsigned int x, unsigned int y) {
  for (unsigned int ry = 0; ry < image.getHeight(); ++ry) {
    for (unsigned int rx = 0; rx < image.getWidth(); ++rx) {
      const auto targetX = x + rx;
      const auto targetY = y + ry;

      setPixel(targetX, targetY, image.getPixel(rx, ry));
    }
  }
}

bool Image::save(const std::string& filename) const {
  const auto output_extension = std::filesystem::path(filename).extension().string();

  int result = 0;
  if (output_extension == ".png") {
    auto pixels_u8 = std::make_unique<unsigned char[]>(len_);
    for (unsigned int i = 0; i < len_; ++i) {
      const float encoded = Color::sRGBEncode(pixels_[i]);
      pixels_u8[i] = std::min(255.0f, std::max(0.0f, 255.0f * encoded + 0.5f));
    }

    const auto stride = 3 * width_;
    result = stbi_write_png(filename.c_str(), width_, height_, 3, pixels_u8.get(), stride);
  } else {
    throw std::runtime_error("Unsupported output image format: " + output_extension);
  }

  return result != 0;
}

Image& Image::operator=(const Image& other) {
  if (&other != this) {
    width_ = other.width_;
    height_ = other.height_;
    len_ = other.len_;

    pixels_ = std::make_unique<float[]>(len_);
    memcpy(pixels_.get(), other.pixels_.get(), len_ * sizeof(float));
  }

  return *this;
}

Image& Image::operator=(Image&& other) {
  if (&other != this) {
    width_ = other.width_;
    height_ = other.height_;
    len_ = other.len_;
    pixels_ = std::move(other.pixels_);
  }

  return *this;
}
