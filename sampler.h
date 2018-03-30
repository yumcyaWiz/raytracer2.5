#ifndef SAMPLER_H
#define SAMPLER_H
#include <random>
#include "vec2.h"
#include "vec3.h"
inline Vec2 sampleDisk(const Vec2& u) {
    const float r = std::sqrt(u.x);
    const float theta = 2 * M_PI * u.y;
    return Vec2(r * std::cos(theta), r * std::sin(theta));
}
inline Vec3 sampleHemisphere(const Vec2& u) {
    const float z = u.x;
    const float r = std::sqrt(std::max(0.0f, 1.0f - z*z));
    const float phi = 2.0f*M_PI*u.y;
    return Vec3(r*std::cos(phi), z, r*std::sin(phi));
}
inline Vec3 sampleCosineHemisphere(const Vec2& u) {
    const Vec2 d = sampleDisk(u);
    const float z = std::sqrt(std::max(0.0f, 1.0f - u.x));
    return Vec3(d.x, z, d.y);
    /*
    const float theta = std::acos(std::sqrt(1.0f - u.x));
    const float phi = 2*M_PI*u.y;
    return Vec3(std::cos(phi)*std::sin(theta), std::cos(theta), std::sin(phi)*std::sin(theta));
    */
}
inline Vec3 sampleNCosineHemisphere(const Vec2& u, float alpha) {
    const float theta = std::acos(std::pow(1.0f - u.x, 1.0f/(alpha + 1.0f)));
    const float phi = 2*M_PI*u.y;
    return Vec3(std::cos(phi)*std::sin(theta), std::cos(theta), std::sin(phi)*std::sin(theta));
}


class Sampler {
    public:

        Sampler() {};

        virtual float getNext() = 0;
        virtual Vec2 getNext2D() = 0;
};


enum class RNG_TYPE {
    MT,
    MINSTD
};


class UniformSampler : public Sampler {
    public:
        std::mt19937 mt;
        std::minstd_rand minstd;
        std::random_device rnd_dev;
        std::uniform_real_distribution<> rnd;
        RNG_TYPE type;

        UniformSampler() {};
        UniformSampler(RNG_TYPE _type) : type(_type) {
            mt.seed(rnd_dev());
            minstd.seed(rnd_dev());
            rnd = std::uniform_real_distribution<>(0.0f, 1.0f);
        };

        float getNext() {
            if(type == RNG_TYPE::MT)
                return rnd(mt);
            else
                return rnd(minstd);
        };
        Vec2 getNext2D() {
            return Vec2(getNext(), getNext());
        };
};
#endif
