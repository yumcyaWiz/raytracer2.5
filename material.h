#ifndef MATERIAL_H
#define MATERIAL_H
#include "vec2.h"
#include "vec3.h"
#include "sampler.h"
//マテリアルクラス
//物体のマテリアル(BRDF, BTDF)を記述する
//主な役割はBRDFの計算と、BRDFによる次のレイの方向のサンプリング
//マテリアル座標系は法線が(0, 1, 0)となるように記述される
//こうすることでコサインの計算とか、方向のサンプリングが簡単になる
//wi, woは基本的にローカル座標系で記述されているものとする


//Thetaは法線からの角度
inline float cosTheta(const Vec3& w) {
    return w.y;
}
inline float absCosTheta(const Vec3& w) {
    return std::abs(w.y);
}
inline float cos2Theta(const Vec3& w) {
    return w.y*w.y;
}
inline float sinTheta(const Vec3& w) {
    return std::sqrt(1.0f - cos2Theta(w));
}
inline float sin2Theta(const Vec3& w) {
    return 1.0f - cos2Theta(w);
}


inline Vec3 reflect(const Vec3& w, const Vec3& n) {
    return -w + 2.0f*dot(w, n)*n;
}
inline bool refract(const Vec3& wi, Vec3& wt, const Vec3& n, float ior1, float ior2) {
    float eta = ior1/ior2;
    //入射角のコサイン
    float cosThetaI = dot(wi, n);
    //入射角のサインの二乗
    float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI*cosThetaI);
    //屈折角のサインの二乗
    float sin2ThetaT = eta*eta*sin2ThetaI;
    //全反射
    if(sin2ThetaT >= 1.0f) return false;
    float cosThetaT = std::sqrt(1.0f - sin2ThetaT);
    //屈折方向の計算
    wt = eta*(-wi) + (eta*cosThetaI - cosThetaT)*n;
    return true;
}


//Schlick近似によるフレネル項の計算
inline float fresnel(const Vec3& w, const Vec3& n, float ior1, float ior2) {
    float f0 = std::pow((ior1 - ior2)/(ior1 + ior2), 2.0f);
    return f0 + (1.0f - f0)*std::pow(1.0f - dot(w, n), 5.0f);
}


enum class MATERIAL_TYPE {
    DIFFUSE,
    SPECULAR,
    GLOSSY
};


class Material {
    public:
        const MATERIAL_TYPE type;

        Material(const MATERIAL_TYPE& _type) : type(_type) {};

        //BRDFを計算する ローカル座標系の方向ベクトルを受け取る
        virtual RGB f(const Vec3& wo, const Vec3& wi) const = 0;
        //BRDFを計算し、BRDFに比例した方向のサンプリングを行う
        virtual RGB sample(const Vec3& wo, Vec3& wi, Sampler& sampler, float &pdf) const = 0;
};


class Lambert : public Material {
    public:
        const RGB reflectance;

        Lambert(const RGB& _reflectance) : Material(MATERIAL_TYPE::DIFFUSE), reflectance(_reflectance) {};

        RGB f(const Vec3& wo, const Vec3& wi) const {
            return reflectance/M_PI;
        };
        RGB sample(const Vec3& wo, Vec3& wi, Sampler& sampler, float &pdf) const {
            //コサインに比例した半球サンプリング
            Vec2 u = sampler.getNext2D();
            wi = sampleCosineHemisphere(u);
            //入射レイのpdf
            pdf = absCosTheta(wi)/M_PI;
            return f(wo, wi);
        };
};


class Mirror : public Material {
    public:
        const float reflectance;
        const MATERIAL_TYPE type = MATERIAL_TYPE::SPECULAR;

        Mirror(const float _reflectance) : Material(MATERIAL_TYPE::SPECULAR), reflectance(_reflectance) {};

        RGB f(const Vec3& wo, const Vec3& wi) const {
            return RGB(0.0f);
        };
        RGB sample(const Vec3& wo, Vec3& wi, Sampler& sampler, float &pdf) const {
            pdf = 1.0f;
            wi = reflect(wo, Vec3(0, 1, 0));
            return 1.0f/absCosTheta(wi)*RGB(1.0f)*reflectance;
        };
};


class Phong : public Material {
    public:
        const Vec3 reflectance;
        const float kd;
        const float alpha;

        Phong(const Vec3& _reflectance, float _kd, float _alpha) : Material(MATERIAL_TYPE::GLOSSY), reflectance(_reflectance), kd(_kd), alpha(_alpha) {};

        RGB f(const Vec3& wo, const Vec3& wi) const {
            //ハーフベクトル
            const Vec3 wh = normalize(wo + wi);
            return kd*reflectance/M_PI + (1.0f - kd)*RGB(1.0f)*(alpha + 2.0f)/(2.0f*M_PI) * std::pow(absCosTheta(wh), alpha);
        };
        RGB sample(const Vec3& wo, Vec3& wi, Sampler& sampler, float &pdf) const {
            Vec2 u = sampler.getNext2D();
            //diffuse
            if(sampler.getNext() < kd) {
                //コサインに比例した半球サンプリング
                wi = sampleCosineHemisphere(u);
                //入射レイのpdf
                pdf = kd * absCosTheta(wi)/M_PI;
                return f(wo, wi);
            }
            //specular
            else {
                //ハーフベクトルのサンプリング
                Vec3 wh = sampleNCosineHemisphere(u, alpha);
                //ハーフベクトルが裏を向いている場合は黒を返す
                if(dot(wo, wh) < 0.0f) return RGB(0.0f);
                //サンプリング方向は出射レイをハーフベクトルで反射したもの
                wi = reflect(wo, wh);
                //物体表面より下の方向がサンプリングされたら黒を返す
                if(wi.y < 0.0f) return RGB(0.0f);
                //ハーフベクトルのpdf
                float pdf_wh = (alpha + 2.0f)/(2*M_PI) * std::pow(absCosTheta(wh), alpha);
                //入射ベクトルのpdf
                pdf = (1.0f - kd) * pdf_wh/(4.0f*std::abs(dot(wo, wh)) + 1e-6);
                return f(wo, wi);
            }
        };
};


class Glass : public Material {
    public:
        const float ior;

        Glass(float _ior) : Material(MATERIAL_TYPE::SPECULAR), ior(_ior) {};

        RGB f(const Vec3& wo, const Vec3& wi) const {
            return RGB(0.0f);
        };
        RGB sample(const Vec3& wo, Vec3& wi, Sampler& sampler, float &pdf) const {
            float ior1, ior2;
            Vec3 normal;
            //物体に入っているか?
            bool entering = cosTheta(wo) > 0.0f;
            if(entering) {
                ior1 = 1.0f;
                ior2 = ior;
                normal = Vec3(0, 1, 0);
            }
            else {
                ior1 = ior;
                ior2 = 1.0f;
                normal = Vec3(0, -1, 0);
            }

            float eta = ior1/ior2;

            //フレネル反射率
            float fr = fresnel(wo, normal, ior1, ior2);
            //反射
            if(sampler.getNext() < fr) {
                wi = reflect(wo, normal);
                pdf = fr;
                return fr * 1.0f/dot(wi, normal)*RGB(1.0f);
            }
            //屈折
            else {
                if(refract(wo, wi, normal, ior1, ior2)) {
                    pdf = 1.0f - fr;
                    return (1.0 - fr) * eta*eta * 1.0f/dot(wi, -normal)*RGB(1.0f);
                }
                //全反射
                else {
                    wi = reflect(wo, normal);
                    pdf = 1.0f - fr;
                    return (1.0f - fr) * 1.0f/dot(wi, normal)*RGB(1.0f);
                }
            }
        };
};
#endif
