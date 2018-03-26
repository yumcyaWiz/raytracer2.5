#ifndef LIGHT_H
#define LIGHT_H
#include "vec3.h"
#include "shape.h"
class Light {
    public:
        Vec3 lightPos;
        RGB power;

        Light() {};
        Light(const Vec3& _lightPos, const RGB& _power) : lightPos(_lightPos), power(_power) {};

        virtual RGB Le(const Hit& res) const = 0;
        virtual RGB sample(const Hit& res, const Vec2& u, Vec3& wi, float &pdf) const = 0;
};


class PointLight : public Light {
    public:
        PointLight(const Vec3& _lightPos, const RGB& _power) : Light(_lightPos, _power) {};


        RGB Le(const Hit& res) const {
            return power;
        };
        RGB sample(const Hit& res, const Vec2& u, Vec3& wi, float &pdf) const {
            pdf = 1.0f;
            wi = normalize(lightPos - res.hitPos);
            return power;
        };
};


class AreaLight : public Light {
    public:
        std::shared_ptr<Shape> shape;


        AreaLight(const Vec3& _lightPos, const RGB& _power, const std::shared_ptr<Shape> _shape) : Light(_lightPos, _power), shape(_shape) {};

        
        RGB Le(const Hit& res) const {
            return power;
        };
        RGB sample(const Hit& res, const Vec2& u, Vec3& wi, float &pdf) const {
            float point_pdf;
            //Vec3 shapePos = shape->samplePoint(u, point_pdf);
            return power;
        };
};
#endif
