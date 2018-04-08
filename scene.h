#ifndef SCENE_H
#define SCEME_H
#include <vector>
#include <memory>
#include "primitive.h"
#include "accel.h"
#include "light.h"
#include "sky.h"
class Scene {
    public:
        std::vector<std::shared_ptr<Primitive>> prims;
        std::vector<std::shared_ptr<Light>> lights;
        std::shared_ptr<Sky> sky;
        std::shared_ptr<Accel<Primitive>> accel;

        Scene() {};
        Scene(const std::vector<std::shared_ptr<Primitive>>& _prims, const std::vector<std::shared_ptr<Light>>& _lights, std::shared_ptr<Sky> _sky) : prims(_prims), lights(_lights), sky(_sky) {
            accel = std::shared_ptr<Accel<Primitive>>(new BVH<Primitive>(prims, 4, BVH_PARTITION_TYPE::SAH));
        };

        bool intersect(const Ray& ray, Hit& res) const {
            return accel->intersect(ray, res);
        };
};
#endif
