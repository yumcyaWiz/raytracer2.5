#ifndef MATERIAL_H
#define MATERIAL_H
#include "vec2.h"
#include "vec3.h"
#include "sampler.h"
inline float cosTheta(const Vec3& w) {
    return w.y;
}
inline float cos2Theta(const Vec3& w) {
    return w.y*w.y;
}
inline float absCosTheta(const Vec3& w) {
    return std::abs(w.y);
}


class Material {
    public:
        Material() {};

        virtual RGB sample(const Vec3& wo, Vec3& wi, const Vec2& u, float &pdf) const = 0;
};


class Lambert : public Material {
    public:
        const RGB reflectance;

        Lambert(const RGB& _reflectance) : reflectance(_reflectance) {};

        RGB sample(const Vec3& wo, Vec3& wi, const Vec2& u, float &pdf) const {
            wi = sampleCosineHemisphere(u);
            pdf = cosTheta(wi)/M_PI;
            return reflectance;
        };
};
#endif
