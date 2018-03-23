#ifndef RAY_H
#define RAY_H
#include "vec3.h"
class Ray {
    public:
        Vec3 origin;
        Vec3 direction;
        float tmax;
        float tmin;

        Ray() {};
        Ray(const Vec3& _origin, const Vec3& _direction) : origin(_origin), direction(_direction), tmax(10000.0f), tmin(0.0f) {};

        Vec3 operator()(float t) const {
            return origin + t*direction;
        };
};
#endif
