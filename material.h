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


//ワールド座標系の方向ベクトルをマテリアル座標系の方向ベクトルに変換する
//w:ワールド座標系方向ベクトル n:法線 s:dpdu t:dpdv
inline Vec3 worldToLocal(const Vec3& w, const Vec3& n, const Vec3& s, const Vec3& t) {
    return Vec3(dot(s, w), dot(n, w), dot(t, w));
}
inline Vec3 localToWorld(const Vec3& w, const Vec3& n, const Vec3& s, const Vec3& t) {
    return Vec3(s.x*w.x + n.x*w.y + t.x*w.z, s.y*w.x + n.y*w.y + t.y*w.z, s.z*w.x + n.z*w.y + t.z*w.z);
}


inline float cosTheta(const Vec3& w) {
    return w.y;
}
inline float cos2Theta(const Vec3& w) {
    return w.y*w.y;
}
inline float absCosTheta(const Vec3& w) {
    return std::abs(w.y);
}


class Material {
    public:
        Material() {};

        virtual RGB f(const Vec3& wo, const Vec3& wi) const = 0;
        virtual RGB sample(const Vec3& wo, Vec3& wi, const Vec3& n, const Vec3& s, const Vec3& t, const Vec2& u, float &pdf) const = 0;
};


class Lambert : public Material {
    public:
        const RGB reflectance;

        Lambert(const RGB& _reflectance) : reflectance(_reflectance) {};

        RGB f(const Vec3& wo, const Vec3& wi) const {
            return reflectance;
        };
        RGB sample(const Vec3& wo, Vec3& wi, const Vec3& n, const Vec3& s, const Vec3& t, const Vec2& u, float &pdf) const {
            wi = localToWorld(sampleCosineHemisphere(u), n, s, t);
            pdf = cosTheta(wi)/M_PI;
            return reflectance;
        };
};
#endif
