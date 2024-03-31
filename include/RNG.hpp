#pragma once

#include <random>

class RNG {
 public:
  RNG() : random_engine_(std::random_device{}()), distribution_(0.0f, 1.0f) {}
  RNG(size_t seed) : random_engine_(seed), distribution_(0.0f, 1.0f) {}

  std::mt19937 getEngine() { return random_engine_; }

  float getReal() { return distribution_(random_engine_); }
  size_t getInteger(size_t min, size_t max) {
    const auto uniform_real = getReal();
    return static_cast<size_t>(std::floor(min + uniform_real * (max - min)));
  }

 private:
  std::mt19937 random_engine_;
  std::uniform_real_distribution<float> distribution_;
};