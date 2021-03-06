#ifndef LIGHT_H
#define LIGHT_H
#include "vec3.h"
#include "shape.h"


enum class LIGHT_TYPE {
    POINT,
    DIRECTIONAL,
    AREA
};


class Light {
    public:
        RGB power;
        LIGHT_TYPE type;

        Light() {};
        Light(const RGB& _power, const LIGHT_TYPE& _type) : power(_power), type(_type) {};

        virtual RGB Le(const Hit& res) const = 0;
        virtual RGB sample(const Hit& res, Sampler& sampler, Vec3& wi, float &pdf) const = 0;
};


class PointLight : public Light {
    public:
        Vec3 lightPos;

        PointLight(const Vec3& _lightPos, const RGB& _power) : Light(_power, LIGHT_TYPE::POINT), lightPos(_lightPos) {};

        RGB Le(const Hit& res) const {
            return power;
        };
        RGB sample(const Hit& res, Sampler& sampler, Vec3& wi, float &pdf) const {
            const float distance2 = (lightPos - res.hitPos).length2();
            pdf = 1.0f * distance2;
            wi = normalize(lightPos - res.hitPos);
            return power;
        };
};


class DirectionalLight : public Light {
    public:
        Vec3 direction;

        DirectionalLight(const Vec3& _direction, const RGB& _power) : Light(_power, LIGHT_TYPE::DIRECTIONAL), direction(_direction) {};
        
        RGB Le(const Hit& res) const {
            return power;
        };
        RGB sample(const Hit& res, Sampler& sampler, Vec3& wi, float &pdf) const {
            pdf = 1.0f;
            wi = direction;
            return power;
        };
};


class AreaLight : public Light {
    public:
        std::shared_ptr<Shape> shape;


        AreaLight(std::shared_ptr<Shape> _shape, const RGB& _power) : Light(_power, LIGHT_TYPE::AREA), shape(_shape) {};

        
        RGB Le(const Hit& res) const {
            return power;
        };
        RGB sample(const Hit& res, Sampler& sampler, Vec3& wi, float &pdf) const {
            float point_pdf;
            //Primitive上で点をサンプリング
            Vec3 normal;
            Vec3 shapePos = shape->sample(sampler, normal, point_pdf);
            float distance2 = (shapePos - res.hitPos).length2();
            //衝突点からサンプリングされた点に向かう方向ベクトルを生成
            wi = normalize(shapePos - res.hitPos);
            float cos_term = std::max(dot(-wi, normal), 0.0f);
            //面積測度を立体角測度に変換
            pdf = point_pdf * distance2/cos_term;
            return power;
        };
};
#endif
