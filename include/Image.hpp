#pragma once

#include <memory>
#include <vector>

#include "Color.hpp"

class Image {
 public:
  Image(const std::string& filename);
  Image(unsigned int width, unsigned int height);

  Image(const Image& other);
  Image(Image&& other);

  bool save(const std::string& filename) const;

  void clear(const Color& color);
  void drawRectangle(const Color& color, unsigned int x, unsigned int y, unsigned int width,
                     unsigned int height);
  void drawImage(const Image& image, unsigned int x, unsigned int y);

  unsigned int getWidth() const { return width_; }
  unsigned int getHeight() const { return height_; }

  inline Color getPixel(unsigned int x, unsigned int y) const;
  inline void setPixel(unsigned int x, unsigned int y, const Color& color);

  Image& operator=(const Image& other);
  Image& operator=(Image&& other);

  float operator[](unsigned int index) const { return pixels_[index]; }
  float& operator[](unsigned int index) { return pixels_[index]; }

 private:
  unsigned int width_;
  unsigned int height_;
  unsigned int len_;
  std::unique_ptr<float[]> pixels_;
};

Color Image::getPixel(unsigned int x, unsigned int y) const {
  const int index = 3 * (y * width_ + x);

  Color c;
  c.r = pixels_[index];
  c.g = pixels_[index + 1];
  c.b = pixels_[index + 2];

  return c;
}

void Image::setPixel(unsigned int x, unsigned int y, const Color& color) {
  const int index = 3 * (y * width_ + x);

  pixels_[index] = color.r;
  pixels_[index + 1] = color.g;
  pixels_[index + 2] = color.b;
}
