#include <CLI11.hpp>
#include <iostream>

#include "Color.hpp"
#include "Image.hpp"
#include "KMeansClustering.hpp"
#include "RNG.hpp"

int main(int argc, char** argv) {
  CLI::App app{"Image palette generator"};
  argv = app.ensure_utf8(argv);

  std::string input_image_path{};
  app.add_option("-i,--input", input_image_path, "Input image")->required();

  size_t num_clusters = 15;
  app.add_option("-n,--num_clusters", num_clusters, "Number of clusters");

  size_t num_iterations = 10;
  app.add_option("--iters", num_iterations, "Number of clustering iterations");

  std::string color_space_name = "oklab";
  app.add_option("--color_space", color_space_name,
                 "Color space in which clustering will be performed. Available options are: "
                 "linear_srgb, srgb, rgG, xyz, oklab (default)");

  size_t padding = 5;
  app.add_option("--padding", padding, "Padding between elements on output image");

  std::string background_color_str = "0, 0, 0";
  app.add_option("--bg", background_color_str,
                 "Background color for generated visualization. Format: \"r, g, b\"");

  size_t seed = 42;
  app.add_option("--seed", seed, "Seed for random number generator");

  bool random = false;
  app.add_flag(
      "--random", random,
      "Use random device to seed random number generator (seed parameter will be ignored)");

  bool sort_colors = false;
  app.add_flag("--sort_colors", sort_colors, "Sort colors in generated palette");

  bool dont_skip_black = false;
  app.add_flag("--dont_skip_black", dont_skip_black,
               "Will include black pixels in clustering when set to true");

  std::string output_file_name = "palette.png";
  app.add_option("-o,--output", output_file_name, "Output image");

  CLI11_PARSE(app, argc, argv);

  ColorSpace working_color_space = ColorSpace::OKLAB;
  std::transform(color_space_name.begin(), color_space_name.end(), color_space_name.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (color_space_name == "linear_srgb") {
    working_color_space = ColorSpace::sRGBLinear;
  } else if (color_space_name == "srgb") {
    working_color_space = ColorSpace::sRGB;
  } else if (color_space_name == "rgg") {
    working_color_space = ColorSpace::rgG;
  } else if (color_space_name == "xyz") {
    working_color_space = ColorSpace::XYZ;
  } else if (color_space_name == "oklab") {
    working_color_space = ColorSpace::OKLAB;
  } else {
    std::cerr << "ERROR: Unrecognized color space (" << color_space_name
              << ")! Use one of the following: linear_srgb, srgb, rgg, xyz, oklab" << std::endl;
    return 1;
  }

  const auto bg_color_opt = Color::parse_string(background_color_str);

  if (!bg_color_opt.has_value()) {
    std::cerr << "ERROR: Could not parse color: \"" << background_color_str << "\n" << std::endl;
    return 1;
  }

  const auto bg_color = bg_color_opt.value();

  RNG rng{seed};

  if (random) {
    rng = RNG{};
  }

  Image image{input_image_path};
  KMeansClustering clustering{rng, image, num_clusters, working_color_space, !dont_skip_black};

  std::cout << "Clustering...\n";
  clustering.run(num_iterations);

  std::cout << "Saving swatches...\n";

  const float swatch_width =
      static_cast<float>(image.getWidth() - padding * (num_clusters - 1)) / num_clusters;
  const int swatch_height = 2 * swatch_width;

  const int palette_image_width = image.getWidth() + 2 * padding;
  const int palette_image_height = image.getHeight() + swatch_height + 3 * padding;

  Image palette_image(palette_image_width, palette_image_height);
  palette_image.clear(bg_color);

  palette_image.drawImage(image, padding, padding);

  std::cout << "Clusters:\n";
  auto clusters = clustering.get_clusters();

  if (sort_colors) {
    std::sort(clusters.begin(), clusters.end(), [](const Color& color1, const Color& color2) {
      const auto c1_oklab = color1.convertTo(ColorSpace::OKLAB);
      const auto c2_oklab = color2.convertTo(ColorSpace::OKLAB);

      const auto h1 = 0.5f + 0.5f * atan2f(-c1_oklab.g, -c1_oklab.b) / 3.14159265358979323846f;
      const auto h2 = 0.5f + 0.5f * atan2f(-c2_oklab.g, -c2_oklab.b) / 3.14159265358979323846f;

      return std::less<float>()(h1, h2);
    });
  }

  for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
    const int swatch_x = std::ceil(padding + cluster_idx * (swatch_width + padding));
    const int swatch_x2 =
        std::min((int)std::ceil(padding + (cluster_idx + 1) * (swatch_width + padding)),
                 palette_image_width - 1);

    const auto adjusted_width = swatch_x2 - swatch_x - padding;

    const int swatch_y = 2 * padding + image.getHeight();

    const auto swatch_color = clusters[cluster_idx].convertTo(ColorSpace::sRGB);

    palette_image.drawRectangle(swatch_color, swatch_x, swatch_y, adjusted_width, swatch_height);
    std::cout << swatch_color << std::endl;
  }

  if (!palette_image.save(output_file_name)) {
    std::cerr << "ERROR: Failed to save output image!\n";
    return 1;
  }

  return 0;
}
