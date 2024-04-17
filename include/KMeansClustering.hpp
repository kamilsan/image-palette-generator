#pragma once

#include <functional>
#include <random>
#include <vector>

#include "Color.hpp"
#include "Image.hpp"
#include "RNG.hpp"

class KMeansClustering {
 public:
  KMeansClustering(RNG& rng, const Image& image, const size_t num_clusters,
                   const ColorSpace color_space, const bool skip_black);

  void run(const size_t num_iterations);

  size_t num_clusters() const { return clusters_.size(); }
  const std::vector<Color>& get_clusters() const { return clusters_; }

 private:
  void assign_colors_to_clusters();
  void recalculate_cluster_positions();

  std::vector<Color> clusters_;
  std::vector<size_t> cluster_assignments_;
  std::vector<Color> colors_;
  RNG rng_;
};