#ifndef HIT_H
#define HIT_H
#include <memory>
#include "vec2.h"
#include "vec3.h"


class Primitive;


class Hit {
    public:
        float t;
        Vec3 hitPos;
        Vec3 hitNormal;
        Vec2 uv;
        Vec3 dpdu;
        Vec3 dpdv;
        const Primitive* hitPrimitive = nullptr;

        Hit() {};
};
#endif
