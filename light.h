#ifndef LIGHT_H
#define LIGHT_H
#include "vec3.h"
#include "primitive.h"


class Light {
    public:
        RGB power;

        Light() {};
        Light(const RGB& _power) : power(_power) {};

        virtual RGB Le(const Hit& res) const = 0;
        virtual RGB sample(const Hit& res, const Vec2& u, Vec3& wi, float &pdf) const = 0;
};


class PointLight : public Light {
    public:
        Vec3 lightPos;

        PointLight(const Vec3& _lightPos, const RGB& _power) : Light(_power), lightPos(_lightPos) {};

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
        std::shared_ptr<Primitive> prim;


        AreaLight(std::shared_ptr<Primitive> _prim, const RGB& _power) : Light(_power), prim(_prim) {};

        
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
