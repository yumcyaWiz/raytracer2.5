#ifndef SCENE_H
#define SCEME_H
#include <vector>
#include <memory>
#include "primitive.h"
#include "light.h"
class Scene {
    public:
        std::vector<std::shared_ptr<Primitive>> prims;
        std::vector<std::shared_ptr<Light>> lights;
        std::shared_ptr<Primitive> accel;

        Scene(const std::vector<std::shared_ptr<Primitive>>& _prims, const std::vector<std::shared_ptr<Light>>& _lights) : prims(_prims), lights(_lights) {
            accel = std::shared_ptr<Primitive>(new BVH(prims, 4, BVH_PARTITION_TYPE::SAH));
        };

        bool intersect(const Ray& ray, Hit& res) const {
            return accel->intersect(ray, res);
        };
};
#endif
