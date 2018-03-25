#ifndef SAMPLER_H
#define SAMPLER_H
#include <random>
#include "vec2.h"
#include "vec3.h"
class Sampler {
    public:

        Sampler() {};

        virtual float getNext() = 0;
        virtual Vec2 getNext2D() = 0;
};


class UniformSampler : public Sampler {
    public:
        std::mt19937 mt;
        std::random_device rnd_dev;
        std::uniform_real_distribution<> rnd;

        UniformSampler() {
            mt.seed(rnd_dev());
            rnd = std::uniform_real_distribution<>(0.0f, 1.0f);
        };

        float getNext() {
            return rnd(mt);
        };
        Vec2 getNext2D() {
            return Vec2(getNext(), getNext());
        };
};
#endif
