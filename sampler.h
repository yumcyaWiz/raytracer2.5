#ifndef SAMPLER_H
#define SAMPLER_H
#include <random>
#include "vec2.h"
#include "vec3.h"
class Sampler {
    public:

        Sampler() {};

        virtual float getNext() const = 0;
        virtual Vec2 getNext2D() const = 0;
        virtual Vec3 getNext3D() const = 0;
};


class UniformSampler : public Sampler {
    public:
        UniformSampler() {};

        float getNext() {
        };
};
#endif
