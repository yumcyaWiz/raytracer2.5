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

        Scene(const std::vector<std::shared_ptr<Primitive>>& _prims, const std::vector<std::shared_ptr<Light>>& _lights) : prims(_prims), lights(_lights) {};
};
#endif
