#ifndef HIT_H
#define HIT_H
#include "vec3.h"
class Hit {
    public:
        float t;
        Vec3 hitPos;
        Vec3 hitNormal;

        Hit() {};
};
#endif
