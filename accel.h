#ifndef ACCEL_H
#define ACCEL_H
#include <vector>
#include <memory>
#include "primitive.h"
class Accel : public Primitive {
    public:
        std::vector<std::shared_ptr<Primitive>> prims;


        Accel(const std::vector<std::shared_ptr<Primitive>>& _prims) : prims(_prims) {};
};
#endif
