#ifndef RAY_H
#define RAY_H
#include "vec3.h"
class Ray {
    public:
        const Vec3 origin;
        const Vec3 direction;
        mutable float tmax;
        constexpr static float tmin = 1e-5;

        Ray() : origin(Vec3()), direction(Vec3()) {};
        Ray(const Vec3& _origin, const Vec3& _direction) : origin(_origin), direction(_direction), tmax(10000.0f) {};

        Vec3 operator()(float t) const {
            return origin + t*direction;
        };
};
#endif
