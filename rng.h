#ifndef RNG_H
#define RNG_H

#include <random>
#include <chrono>

class RNG {
public:
    RNG() {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937::result_type seed = static_cast<std::mt19937::result_type>(now);
        engine.seed(seed);
    }

    int rand(int min, int max) {
        std::uniform_int_distribution<> distr(min, max);
        return distr(engine);
    }

private:
    std::mt19937 engine; // Mersenne Twister engine
};

#endif