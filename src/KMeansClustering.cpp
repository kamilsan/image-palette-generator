#include "KMeansClustering.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

KMeansClustering::KMeansClustering(RNG& rng, const Image& image, const size_t num_clusters,
                                   const bool skip_black)
    : colors_(), rng_(rng) {
  for (unsigned int y = 0; y < image.getHeight(); ++y) {
    for (unsigned int x = 0; x < image.getWidth(); ++x) {
      const auto& color = image.getPixel(x, y);

      if (skip_black) {
        if (color.r == 0.0f && color.g == 0.0f && color.b == 0.0f) {
          continue;
        }
      }

      colors_.emplace_back(color);
    }
  }

  clusters_.reserve(num_clusters);
  cluster_assignments_ = std::vector<size_t>(colors_.size(), 0);

  std::sample(colors_.begin(), colors_.end(), std::back_inserter(clusters_), num_clusters,
              rng_.getEngine());
}

void KMeansClustering::run(const size_t num_iterations) {
  for (size_t iteration = 0; iteration < num_iterations; ++iteration) {
    assign_colors_to_clusters();
    recalculate_cluster_positions();
  }
}

void KMeansClustering::assign_colors_to_clusters() {
  for (size_t color_idx = 0; color_idx < colors_.size(); ++color_idx) {
    const auto& color = colors_[color_idx];

    float min_distance = std::numeric_limits<float>::max();
    size_t closest_cluster_id = 0;

    for (size_t cluster_idx = 0; cluster_idx < clusters_.size(); ++cluster_idx) {
      const auto distance = color.distance(clusters_[cluster_idx]);

      if (distance < min_distance) {
        min_distance = distance;
        closest_cluster_id = cluster_idx;
      }
    }

    cluster_assignments_[color_idx] = closest_cluster_id;
  }
}

void KMeansClustering::recalculate_cluster_positions() {
  for (size_t cluster_idx = 0; cluster_idx < clusters_.size(); ++cluster_idx) {
    Color new_cluster{};
    size_t colors_in_cluster = 0;

    for (size_t color_idx = 0; color_idx < colors_.size(); ++color_idx) {
      if (cluster_assignments_[color_idx] == cluster_idx) {
        new_cluster += colors_[color_idx];
        colors_in_cluster += 1;
      }
    }

    if (colors_in_cluster > 0) {
      clusters_[cluster_idx] = new_cluster / colors_in_cluster;
    } else {
      std::cout << "WARNING: Empty cluster " << cluster_idx << " will be reinitialized\n";
      const size_t color_idx = rng_.getInteger(0, colors_.size());
      clusters_[cluster_idx] = colors_[color_idx];
    }
  }
}