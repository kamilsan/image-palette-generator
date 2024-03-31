#pragma once

#include <cmath>
#include <iostream>

struct Color {
  Color() : r(0), g(0), b(0) {}
  Color(float rr, float gg, float bb) : r(rr), g(gg), b(bb) {}

  static Color sRGBToLinear(const Color& srgb) {
    return Color{sRGBDecode(srgb.r), sRGBDecode(srgb.g), sRGBDecode(srgb.b)};
  }

  static Color linearTosRGB(const Color& linear_srgb) {
    return Color{sRGBEncode(linear_srgb.r), sRGBEncode(linear_srgb.g), sRGBEncode(linear_srgb.b)};
  }

  static float sRGBEncode(float c) {
    const float y = 1.0f / 2.4f;

    if (c <= 0.0031308f)
      return c * 12.92f;
    else
      return 1.055f * std::pow(c, y) - 0.055f;
  }

  static float sRGBDecode(float c) {
    if (c <= 0.04045f)
      return c / 12.92f;
    else
      return std::pow((c + 0.055f) / 1.055f, 2.4f);
  }

  float distance(const Color& other) const {
    const float dr = r - other.r;
    const float dg = g - other.g;
    const float db = b - other.b;

    return dr * dr + dg * dg + db * db;
  }

  Color operator+(const Color& other) const { return Color{r + other.r, g + other.g, b + other.b}; }

  Color operator/(const float value) const {
    const auto f = 1.0f / value;
    return Color{r * f, g * f, b * f};
  }

  Color& operator+=(const Color& other) {
    r += other.r;
    g += other.g;
    b += other.b;

    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, const Color& color) {
    return os << "[" << color.r << ", " << color.g << ", " << color.b << "]";
  }

  float r;
  float g;
  float b;

 private:
};