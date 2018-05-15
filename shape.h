#ifndef SHAPE_H
#define SHAPE_H
#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "hit.h"
#include "aabb.h"
#include "util.h"
#include "accel.h"
#include "sampler.h"


class Shape {
    public:
        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
        virtual AABB worldBound() const = 0;
        virtual float surfaceArea() const = 0;
        virtual Vec3 sample(Sampler& sampler, Vec3& normal, float &pdf) const = 0;
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
            if(tHit <= ray.tmin) {
                tHit = t1;
                if(tHit > ray.tmax) return false;
            }
            Vec3 hitPos = ray(tHit);
            Vec3 localHitPos = hitPos - center;
            if(localHitPos.x == 0 && localHitPos.z == 0) localHitPos.x -= 1e-5*radius;

            float phi = std::atan2(localHitPos.z, localHitPos.x);
            if(phi < 0) phi += 2*M_PI;
            float theta = std::acos(clamp(localHitPos.y/radius, -1.0f, 1.0f));

            res.t = tHit;
            res.uv = Vec2(phi/(2*M_PI), 1.0f - theta/M_PI);
            Vec3 dpdu = Vec3(-2*M_PI*localHitPos.z, 0, 2*M_PI*localHitPos.x);
            Vec3 dpdv = M_PI * Vec3(localHitPos.y*std::cos(phi), -radius*std::sin(theta), localHitPos.y*std::sin(phi));
            res.dpdu = normalize(dpdu);
            res.dpdv = normalize(dpdv);
            res.hitNormal = normalize(cross(dpdu, dpdv));
            res.hitPos = hitPos;
            return true;
        };
        AABB worldBound() const {
            return AABB(Vec3(-radius) + center, Vec3(radius) + center);
        };

        float surfaceArea() const {
            return 4*M_PI*radius*radius;
        };

        Vec3 sample(Sampler& sampler, Vec3& normal, float &pdf) const {
            Vec2 u = sampler.getNext2D();
            float theta = M_PI*u.x;
            float phi = 2*M_PI*u.y;
            Vec3 samplingPos = center + radius*Vec3(std::cos(phi)*std::sin(theta), std::cos(theta), std::sin(phi)*std::sin(theta));
            normal = normalize(samplingPos - center);
            pdf = 1.0f/(surfaceArea());
            return samplingPos;
        };
};


class Triangle : public Shape {
    public:
        Vec3 p1, p2, p3; //頂点座標
        Vec3 n1, n2, n3; //頂点法線
        Vec2 t1, t2, t3; //頂点UV座標
        Vec3 face_normal; //面法線
        Vec3 dpdu; 
        Vec3 dpdv;
        bool vertex_normal;

        Triangle(const Vec3& _p1, const Vec3& _p2, const Vec3& _p3) : p1(_p1), p2(_p2), p3(_p3) {
            dpdu = normalize(p2 - p1);
            dpdv = normalize(p3 - p1);
            face_normal = normalize(cross(dpdu, dpdv));
            vertex_normal = false;
        };
        Triangle(const Vec3& _p1, const Vec3& _p2, const Vec3& _p3, const Vec3& _n1, const Vec3& _n2, const Vec3& _n3) : p1(_p1), p2(_p2), p3(_p3), n1(_n1), n2(_n2), n3(_n3) {
            dpdu = normalize(p2 - p1);
            dpdv = normalize(p3 - p1);
            vertex_normal = true;
        };

        bool intersect(const Ray& ray, Hit& res) const {
            const float eps = 1e-6;
            const Vec3 edge1 = p2 - p1;
            const Vec3 edge2 = p3 - p1;
            const Vec3 h = cross(ray.direction, edge2);
            const float a = dot(edge1, h);
            if(a >= -eps && a <= eps)
                return false;
            const float f = 1.0f/a;
            const Vec3 s = ray.origin - p1;
            const float u = f*dot(s, h);
            if(u < 0.0f || u > 1.0f)
                return false;
            const Vec3 q = cross(s, edge1);
            const float v = f*dot(ray.direction, q);
            if(v < 0.0f || u + v > 1.0f)
                return false;
            float t = f*dot(edge2, q);
            if(t <= ray.tmin || t > ray.tmax)
                return false;
            
            res.t = t;
            res.hitPos = ray(t);
            if(vertex_normal) {
                res.hitNormal = normalize((1.0f - u - v)*n1 + u*n2 + v*n3);
            }
            else {
                res.hitNormal = face_normal;
            }
            res.uv = Vec2(u, v);
            res.dpdu = dpdu;
            res.dpdv = dpdv;

            return true;
        };

        AABB worldBound() const {
            return AABB(min(p1, min(p2, p3)), max(p1, max(p2, p3)));
        };

        float surfaceArea() const {
            return 0.5f * std::abs(cross(p2 - p1, p3 - p1).length());
        };

        Vec3 sample(Sampler& sampler, Vec3& normal, float &pdf) const {
            Vec2 u = sampleTriangle(sampler.getNext2D());
            Vec3 samplePos = (1.0f - u.x - u.y)*p1 + u.x*p2 + u.y*p3;
            if(vertex_normal)
                normal = normalize((1.0f - u.x - u.y)*n1 + u.x*n2 + u.y*n3);
            else
                normal = face_normal;
            pdf = 1.0f;
            return samplePos;
        };
};


class Polygon : public Shape {
    public:
        std::vector<std::shared_ptr<Triangle>> triangles;
        std::shared_ptr<Accel<Triangle>> accel;

        Polygon(const std::vector<std::shared_ptr<Triangle>>& _triangles) : triangles(_triangles) {
            accel = std::shared_ptr<Accel<Triangle>>(new BVH<Triangle>(triangles, 4, BVH_PARTITION_TYPE::SAH));
        };

        bool intersect(const Ray& ray, Hit& res) const {
            return accel->intersect(ray, res);
        };

        AABB worldBound() const {
            return accel->worldBound();
        };

        float surfaceArea() const {
            float area = 0.0f;
            for(const auto& triangle : triangles) {
                area += triangle->surfaceArea();
            }
            return area;
        };

        Vec3 sample(Sampler& sampler, Vec3& normal, float &pdf) const {
            int tri_num = std::floor(triangles.size()*sampler.getNext());
            if(tri_num == triangles.size()) tri_num--;
            Vec3 samplePos = triangles[tri_num]->sample(sampler, normal, pdf);
            pdf = 1.0f/this->surfaceArea();
            return samplePos;
        };
};
#endif
