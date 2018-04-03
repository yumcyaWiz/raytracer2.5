#ifndef CAMERA_H
#define CAMERA_H
#include <cmath>
#include <memory>
#include "vec3.h"
#include "ray.h"
#include "sampler.h"
class Camera {
    public:
        Vec3 camPos;
        Vec3 camForward;
        Vec3 camRight;
        Vec3 camUp;

        Camera(const Vec3& _camPos, const Vec3& _camForward) : camPos(_camPos) {
            camForward = normalize(_camForward);
            camRight = normalize(cross(camForward, Vec3(0, 1, 0)));
            camUp = normalize(cross(camRight, camForward));
        };

        virtual Ray getRay(float u, float v, float &w, Sampler& sampler) const = 0;
};


class PinholeCamera : public Camera {
    public:
        float focus;

        PinholeCamera(const Vec3& _camPos, const Vec3& _camForward, float fov) : Camera(_camPos, _camForward) {
            focus = 1.0f/std::tan(fov/2.0f);
        };

        Ray getRay(float u, float v, float &w, Sampler& sampler) const {
            Vec3 rayDir = normalize(focus*camForward + u*camRight + v*camUp);
            w = std::pow(dot(camForward, rayDir), 4.0f);
            return Ray(camPos, rayDir);
        };
};


class ThinLensCamera : public Camera {
    public:
        //センサー面からレンズ面までの距離
        float lensDistance;
        //フォーカスをあわせる位置
        Vec3 focusPoint;
        //主点からオブジェクトプレーンまでの距離
        float objectDistance;
        //焦点距離
        float focalLength;
        //F値
        float Fnumber;
        //レンズの半径
        float lensRadius;
        //レンズ中心位置
        Vec3 lensCenterPos;

        ThinLensCamera(const Vec3& _camPos, const Vec3& _camForward, float _lensDistance, const Vec3& _focusPoint, float _Fnumber) : Camera(_camPos, _camForward), lensDistance(_lensDistance), focusPoint(_focusPoint), Fnumber(_Fnumber) {
            objectDistance = (focusPoint - camPos).length() - lensDistance;
            focalLength = 1.0f/(1.0f/lensDistance + 1.0f/objectDistance);
            std::cout << "lensDistance:" << lensDistance << std::endl;
            std::cout << "objectDistance:" << objectDistance << std::endl;
            std::cout << "focalLength:" << focalLength << std::endl;
            lensRadius = 0.5f * focalLength/Fnumber;
            std::cout << "lensRadius:" << lensRadius << std::endl;
            lensCenterPos = camPos + lensDistance*camForward;
            std::cout << "lensCenterPos:" << lensCenterPos << std::endl;
        };

        Ray getRay(float u, float v, float &w, Sampler& sampler) const {
            //像が逆になるので修正する
            u = -u;
            v = -v;
            //センサー面上の位置
            Vec3 sensorPos = camPos + u*camRight + v*camUp;
            //レンズ上の位置をサンプリング
            Vec2 l = sampleDisk(sampler.getNext2D());
            Vec3 lensPos = lensCenterPos + lensRadius*(l.x*camRight + l.y*camUp);

            //センサー面からレンズ中心までの方向ベクトル
            Vec3 sensorToLensCenter = normalize(lensCenterPos - sensorPos);
            //センサー面からオブジェクト面までの距離
            float sensorObjectDistance = (lensDistance + objectDistance)/dot(camForward, sensorToLensCenter);
            //オブジェクト面での位置
            Vec3 objectPos = sensorPos + sensorObjectDistance*sensorToLensCenter;

            //センサー面からレンズ面までの方向ベクトル
            Vec3 sensorToLensPos = normalize(lensPos - sensorPos);
            //センサー面からレンズ面までの距離の二乗
            float sensorLensDistance2 = (lensPos - sensorPos).length2();
            //寄与係数の計算
            w = lensDistance*lensDistance*std::pow(dot(camForward, sensorToLensPos), 2.0f)/sensorLensDistance2;

            Vec3 rayDir = normalize(objectPos - lensPos);
            return Ray(lensPos, rayDir);
        };
};
#endif
