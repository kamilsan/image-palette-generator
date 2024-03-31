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

  size_t padding = 5;
  app.add_option("--padding", padding, "Padding between elements on output image");

  size_t seed = 42;
  app.add_option("--seed", seed, "Seed for random number generator");

  bool random = false;
  app.add_flag(
      "--random", random,
      "Use random device to seed random number generator (seed parameter will be ignored)");

  bool dont_skip_black = false;
  app.add_flag("--dont_skip_black", dont_skip_black,
               "Will include black pixels in clustering when set to true");

  std::string output_file_name = "palette.png";
  app.add_option("-o,--output", output_file_name, "Output image");

  CLI11_PARSE(app, argc, argv);

  RNG rng{seed};

  if (random) {
    rng = RNG{};
  }

  Image image{input_image_path};
  KMeansClustering clustering{rng, image, num_clusters, !dont_skip_black};

  std::cout << "Clustering...\n";
  clustering.run(num_iterations);

  std::cout << "Saving swatches...\n";

  const float swatch_width =
      static_cast<float>(image.getWidth() - padding * (num_clusters - 1)) / num_clusters;
  const int swatch_height = 2 * swatch_width;

  const int palette_image_width = image.getWidth() + 2 * padding;
  const int palette_image_height = image.getHeight() + swatch_height + 3 * padding;

  Image palette_image(palette_image_width, palette_image_height);
  palette_image.clear(Color{255, 255, 255});

  palette_image.drawImage(image, padding, padding);

  std::cout << "Clusters:\n";
  const auto& clusters = clustering.get_clusters();

  for (size_t cluster_idx = 0; cluster_idx < clusters.size(); ++cluster_idx) {
    const int swatch_x = std::ceil(padding + cluster_idx * (swatch_width + padding));
    const int swatch_y = 2 * padding + image.getHeight();

    const auto& swatch_color = clusters[cluster_idx];

    palette_image.drawRectangle(swatch_color, swatch_x, swatch_y, swatch_width, swatch_height);
    std::cout << Color::linearTosRGB(swatch_color) << "\n";
  }

  if (!palette_image.save(output_file_name)) {
    std::cerr << "ERROR: Failed to save output image!\n";
    return 1;
  }

  return 0;
}
