// Copyright (c) 2020 Björn Ottosson
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>

enum class ColorSpace { sRGB, sRGBLinear, rgG, XYZ, OKLAB };

struct Color {
  Color(ColorSpace color_space = ColorSpace::sRGB) : r(0), g(0), b(0), color_space_(color_space) {}
  Color(float rr, float gg, float bb, ColorSpace color_space = ColorSpace::sRGB)
      : r(rr), g(gg), b(bb), color_space_(color_space) {}

  Color convertTo(ColorSpace color_space) const {
    if (color_space_ == color_space) {
      return *this;
    } else if (color_space_ == ColorSpace::sRGBLinear) {
      return convertLinearTo(color_space);
    } else {
      return toLinearsRGB().convertLinearTo(color_space);
    }
  }

  static std::optional<Color> parse_string(const std::string& str) {
    const auto first_comma = str.find(',');

    if (first_comma == str.npos) {
      return {};
    }

    auto second_comma = str.find(',', first_comma + 1);
    if (second_comma == str.npos) {
      return {};
    }

    const auto r_str = trim_string(str.substr(0, first_comma));
    const auto g_str = trim_string(str.substr(first_comma + 1, second_comma - first_comma - 1));
    const auto b_str = trim_string(str.substr(second_comma + 1));

    try {
      const int r = std::stoi(r_str);
      const int g = std::stoi(g_str);
      const int b = std::stoi(b_str);

      if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        return {};
      }

      return Color{r / 255.0f, g / 255.0f, b / 255.0f};
    } catch (std::exception&) {
      return {};
    }
  }

  ColorSpace getColorSpace() const { return color_space_; }

  float distance(const Color& other) const {
    const float dr = r - other.r;
    const float dg = g - other.g;
    const float db = b - other.b;

    return dr * dr + dg * dg + db * db;
  }

  float operator[](const size_t index) const {
    if (index == 0) {
      return r;
    } else if (index == 1) {
      return g;
    } else if (index == 2) {
      return b;
    } else {
      throw std::out_of_range("Attempting to access color component with index out of range (" +
                              std::to_string(index) + ")!");
    }
  }

  float& operator[](const size_t index) {
    if (index == 0) {
      return r;
    } else if (index == 1) {
      return g;
    } else if (index == 2) {
      return b;
    } else {
      throw std::out_of_range("Attempting to access color component with index out of range (" +
                              std::to_string(index) + ")!");
    }
  }

  Color operator+(const Color& other) const {
    return Color{r + other.r, g + other.g, b + other.b, color_space_};
  }

  Color operator/(const float value) const {
    const auto f = 1.0f / value;
    return Color{r * f, g * f, b * f, color_space_};
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
  ColorSpace color_space_;

  static std::string trim_string(const std::string& str) {
    size_t start_idx = 0;
    while (start_idx < str.size() && str[start_idx] == ' ') {
      start_idx += 1;
    }

    size_t end_idx = str.size() - 1;
    while (end_idx > 0 && str[end_idx] == ' ') {
      end_idx -= 1;
    }

    return str.substr(start_idx, end_idx - start_idx + 1);
  }

  Color convertLinearTo(ColorSpace color_space) const {
    switch (color_space) {
      case ColorSpace::sRGB: {
        return Color{sRGBEncode(r), sRGBEncode(g), sRGBEncode(b), ColorSpace::sRGB};
      }
      case ColorSpace::sRGBLinear: {
        return *this;
      }
      case ColorSpace::rgG: {
        const auto normalizing_factor = 1.0f / (r + g + b + 0.0001f);
        return Color{r * normalizing_factor, g * normalizing_factor, g, ColorSpace::rgG};
      }
      case ColorSpace::XYZ: {
        const auto x = 0.4124108464885388f * r + 0.3575845678529519f * g + 0.18045380393360833f * b;
        const auto y =
            0.21264934272065283f * r + 0.7151691357059038f * g + 0.07218152157344333f * b;
        const auto z =
            0.019331758429150258f * r + 0.11919485595098397f * g + 0.9503900340503373f * b;

        return Color{x, y, z, ColorSpace::XYZ};
      }
      case ColorSpace::OKLAB: {
        // https://bottosson.github.io/posts/oklab/
        const auto l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
        const auto m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
        const auto s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

        const auto l_ = std::cbrtf(l);
        const auto m_ = std::cbrtf(m);
        const auto s_ = std::cbrtf(s);

        const auto OKL = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
        const auto OKA = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
        const auto OKB = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;

        return Color{OKL, OKA, OKB, ColorSpace::OKLAB};
      }
      default:
        throw std::runtime_error("Unsupported target color space!");
    }
  }

  Color toLinearsRGB() const {
    switch (color_space_) {
      case ColorSpace::sRGB: {
        return Color{sRGBDecode(r), sRGBDecode(g), sRGBDecode(b), ColorSpace::sRGBLinear};
      }
      case ColorSpace::sRGBLinear: {
        return *this;
      }
      case ColorSpace::rgG: {
        // Confusing bit because b = G
        const auto factor = b / g;
        return Color{r * factor, b, (1.0f - r - g) * factor, ColorSpace::sRGBLinear};
      }
      case ColorSpace::XYZ: {
        const auto x = r;
        const auto y = g;
        const auto z = b;

        const auto rr = 3.240812398895283f * x - 1.5373084456298136f * y - 0.4985865229069666f * z;
        const auto gg =
            -0.9692430170086407f * x + 1.8759663029085742f * y + 0.04155503085668564f * z;
        const auto bb =
            0.055638398436112804f * x - 0.20400746093241362f * y + 1.0571295702861434f * z;

        return Color{rr, gg, bb, ColorSpace::sRGBLinear};
      }
      case ColorSpace::OKLAB: {
        const auto OKL = r;
        const auto OKa = g;
        const auto OKb = b;

        const auto l_ = OKL + 0.3963377774f * OKa + 0.2158037573f * OKb;
        const auto m_ = OKL - 0.1055613458f * OKa - 0.0638541728f * OKb;
        const auto s_ = OKL - 0.0894841775f * OKa - 1.2914855480f * OKb;

        const auto l = l_ * l_ * l_;
        const auto m = m_ * m_ * m_;
        const auto s = s_ * s_ * s_;

        const auto rr = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
        const auto gg = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
        const auto bb = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

        return Color{rr, gg, bb, ColorSpace::sRGBLinear};
      }
      default:
        throw std::runtime_error("Unsupported target color space!");
    }
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
};