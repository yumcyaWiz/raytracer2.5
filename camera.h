#ifndef CAMERA_H
#define CAMERA_H
#include <cmath>
#include <memory>
#include "vec3.h"
#include "ray.h"
#include "sampler.h"
#include "film.h"
class Camera {
    public:
        Vec3 camPos;
        Vec3 camForward;
        Vec3 camRight;
        Vec3 camUp;
        std::shared_ptr<Film> film;

        Camera(const Vec3& _camPos, const Vec3& _camForward, std::shared_ptr<Film> _film) : camPos(_camPos) {
            camForward = normalize(_camForward);
            camRight = normalize(cross(camForward, Vec3(0, 1, 0)));
            camUp = normalize(cross(camRight, camForward));
            film = _film;
        };

        virtual Ray getRay(float u, float v, float &w, Sampler& sampler, bool isLeft = true) const = 0;
};


class PinholeCamera : public Camera {
    public:
        float focus;

        PinholeCamera(const Vec3& _camPos, const Vec3& _camForward, std::shared_ptr<Film> _film, float fov) : Camera(_camPos, _camForward, _film) {
            focus = 1.0f/std::tan(fov/2.0f);
        };

        Ray getRay(float u, float v, float &w, Sampler& sampler, bool isLeft = true) const {
            Vec3 rayDir = normalize(focus*camForward + u*camRight + v*camUp);
            w = std::pow(dot(camForward, rayDir), 4.0f);
            return Ray(camPos, rayDir);
        };
};


class FullDegreeCamera : public Camera {
    public:
        FullDegreeCamera(const Vec3& _camPos, const Vec3& _camForward, std::shared_ptr<Film> _film) : Camera(_camPos, _camForward, _film) {
        };

        Ray getRay(float u, float v, float &w, Sampler& sampler, bool isLeft = true) const {
            float phi = M_PI * u;
            float theta = M_PI/2 * (-v + 1.0f);
            float x = std::sin(theta)*std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta)*std::sin(phi);
            Vec3 rayDir = normalize(x*camRight + y*camUp + z*camForward);
            w = 1.0f;
            return Ray(camPos, rayDir);
        };
};


class ODSCamera : public Camera {
    public:
        float IPD;

        ODSCamera(const Vec3& _camPos, const Vec3& _camForward, std::shared_ptr<Film> _film, float _IPD) : Camera(_camPos, _camForward, _film), IPD(_IPD)  {};

        Ray getRay(float u, float v, float &w, Sampler& sampler, bool isLeft) const {
            float phi = M_PI * u;
            float theta = M_PI/2 * (-v + 1.0f);
            float x = std::sin(theta)*std::cos(phi);
            float y = std::cos(theta);
            float z = std::sin(theta)*std::sin(phi);
            Vec3 rayDir = normalize(x*camRight + y*camUp + z*camForward);

            Vec3 dp = Vec3(std::cos(phi), 0, std::sin(phi));
            Vec3 rayOrigin;
            if(isLeft) {
                rayOrigin = camPos - IPD/2*dp;
            }
            else {
                rayOrigin = camPos + IPD/2*dp;
            }
            return Ray(rayOrigin, rayDir);
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

        ThinLensCamera(const Vec3& _camPos, const Vec3& _camForward, std::shared_ptr<Film> _film, float _lensDistance, const Vec3& _focusPoint, float _Fnumber) : Camera(_camPos, _camForward, _film), lensDistance(_lensDistance), focusPoint(_focusPoint), Fnumber(_Fnumber) {
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

        Ray getRay(float u, float v, float &w, Sampler& sampler, bool isLeft = true) const {
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
