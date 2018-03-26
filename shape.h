#ifndef SHAPE_H
#define SHAPE_H
#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "hit.h"
#include "aabb.h"
class Shape {
    public:
        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
        virtual AABB worldBound() const = 0;
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
        AABB worldBound() const {
            return AABB(Vec3(-radius), Vec3(radius));
        };
};


class Triangle : public Shape {
    public:
        Vec3 p1, p2, p3;
        Vec3 normal;
        Vec3 dpdu;
        Vec3 dpdv;

        Triangle(const Vec3& _p1, const Vec3& _p2, const Vec3& _p3) : p1(_p1), p2(_p2), p3(_p3) {
            dpdu = normalize(p2 - p1);
            dpdv = normalize(p3 - p1);
            normal = normalize(cross(p2 - p1, p3 - p1));
        };

        bool intersect(const Ray& ray, Hit& res) const {
            const float eps = 0.000001;
            const Vec3 edge1 = p2 - p1;
            const Vec3 edge2 = p3 - p1;
            const Vec3 h = cross(ray.direction, edge2);
            const float a = dot(edge1, h);
            if(a >= -eps && a <= eps)
                return false;
            const float f = 1/a;
            const Vec3 s = ray.origin - p1;
            const float u = f*dot(s, h);
            if(u < 0.0 || u > 1.0)
                return false;
            const Vec3 q = cross(s, edge1);
            const float v = f*dot(ray.direction, q);
            if(v < 0.0 || u + v > 1.0)
                return false;
            const float t = f*dot(edge2, q);
            if(t <= ray.tmin || t > ray.tmax)
                return false;
            
            res.t = t;
            res.hitPos = ray(t);
            res.hitNormal = normal;
            res.uv = Vec2(u, v);
            res.dpdu = dpdu;
            res.dpdv = dpdv;
            return true;
        };

        AABB worldBound() const {
            return AABB(min(p1, min(p2, p3)), max(p1, max(p2, p3)));
        };
};
#endif
