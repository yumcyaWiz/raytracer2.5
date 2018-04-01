#ifndef CAMERA_H
#define CAMERA_H
#include <cmath>
#include "vec3.h"
#include "ray.h"
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

        virtual Ray getRay(float u, float v, float &w) const = 0;
};


class PinholeCamera : public Camera {
    public:
        float focus;

        PinholeCamera(const Vec3& _camPos, const Vec3& _camForward, float fov) : Camera(_camPos, _camForward) {
            focus = 1.0f/std::tan(fov/2.0f);
        };

        Ray getRay(float u, float v, float &w) const {
            Vec3 rayDir = normalize(focus*camForward + u*camRight + v*camUp);
            w = std::pow(dot(camForward, rayDir), 4.0f);
            return Ray(camPos, rayDir);
        };
};
#endif
