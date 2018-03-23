#ifndef SHAPE_H
#define SHAPE_H
#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "hit.h"
class Shape {
    public:
        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
};


class Sphere : public Shape {
    public:
        Vec3 center;
        float radius;

        Sphere(const Vec3& _center, float _radius) : center(_center), radius(_radius) {};

        bool intersect(const Ray& ray, Hit& res) const {
            const float b = dot(ray.direction, ray.origin - center);
            const float c = (ray.origin - center).length2() - radius*radius;
            const float D = b*b - c;
            if(D < 0) return false;
            const float t0 = -b - std::sqrt(D);
            const float t1 = -b + std::sqrt(D);
            if(t0 > ray.tmax || t1 <= ray.tmin) return false;
            float tHit = t0;
            if(t1 < ray.tmax) {
                tHit = t1;
                if(tHit > ray.tmax) return false;
            }

            res.t = tHit;
            res.hitPos = ray(tHit);
            res.hitNormal = normalize(res.hitPos - center);
            return true;
        };
};
#endif
