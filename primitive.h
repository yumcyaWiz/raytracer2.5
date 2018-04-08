#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include <memory>
#include "aabb.h"
#include "material.h"
#include "shape.h"


class Light;


class Primitive {
    public:
        std::shared_ptr<Material> material;
        std::shared_ptr<Light> areaLight;

        Primitive() {};
        Primitive(const std::shared_ptr<Material> _material, std::shared_ptr<Light> _areaLight) : material(_material), areaLight(_areaLight) {};

        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
        virtual AABB worldBound() const = 0;
        virtual Vec3 sample(const Vec2& u, Vec3& normal, float &pdf) const = 0;
};


class GeometricPrimitive : public Primitive {
    public:
        std::shared_ptr<Shape> shape;


        GeometricPrimitive(const std::shared_ptr<Material> _material, std::shared_ptr<Light> _areaLight, std::shared_ptr<Shape> _shape) : Primitive(_material, _areaLight), shape(_shape) {};


        bool intersect(const Ray& ray, Hit& res) const {
            if(!shape->intersect(ray, res)) return false;
            ray.tmax = res.t;
            res.hitPrimitive = this;
            return true;
        };

        AABB worldBound() const {
            return shape->worldBound();
        };

        Vec3 sample(const Vec2& u, Vec3& normal, float &pdf) const {
            return shape->sample(u, normal, pdf);
        };
};
#endif
