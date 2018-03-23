#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include <memory>
#include "shape.h"
class Primitive {
    public:
        std::shared_ptr<Shape> shape;


        Primitive(const std::shared_ptr<Shape> _shape) : shape(_shape) {};


        virtual bool intersect(const Ray& ray, Hit& res) const {
            if(!shape->intersect(ray, res)) return false;
            if(res.t < ray.tmax)
                ray.tmax = res.t;
            return true;
        };
};
#endif
