#include "types.h"

// ------------ RNG ------------
Rng::Rng(uint32_t seed) : gen(seed) {}

float Rng::uniform01() {
static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
return dist(gen);
}
int Rng::rangeInt(int minInclusive, int maxExclusive) {
    return std::uniform_int_distribution<int>(minInclusive, maxExclusive - 1)(gen);
}