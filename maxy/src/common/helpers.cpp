#include <random>

float random_float(float lo, float hi) {
    static std::mt19937 rng{std::random_device{}()};
    return std::uniform_real_distribution<float>{lo, hi}(rng);
}

int random_int(int lo, int hi) {
    static std::mt19937 rng{std::random_device{}()};
    return std::uniform_int_distribution<int>{lo, hi}(rng);
}
