#ifndef SAMPLER_H
#define SAMPLER_H
#include <random>
#include "vec2.h"
#include "vec3.h"
inline Vec2 sampleDisk(const Vec2& u) {
    float r = std::sqrt(u.x);
    float theta = 2 * M_PI * u.y;
    return Vec2(r * std::cos(theta), r * std::sin(theta));
}
inline Vec3 sampleCosineHemisphere(const Vec2& u) {
    Vec2 d = sampleDisk(u);
    float z = std::sqrt(std::max(0.0f, 1.0f - d.x*d.x - d.y*d.y));
    return Vec3(d.x, d.y, z);
}


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
