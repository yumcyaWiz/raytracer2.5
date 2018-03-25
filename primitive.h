#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include <memory>
#include "shape.h"
#include "aabb.h"
class Primitive {
    public:
        Primitive() {};
        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
        virtual AABB worldBound() const = 0;
};


class GeometricPrimitive : public Primitive {
    public:
        std::shared_ptr<Shape> shape;


        GeometricPrimitive(const std::shared_ptr<Shape> _shape) : shape(_shape) {};


        bool intersect(const Ray& ray, Hit& res) const {
            if(!shape->intersect(ray, res)) return false;
            ray.tmax = res.t;
            return true;
        };


        AABB worldBound() const {
            return shape->worldBound();
        };
};
#endif
