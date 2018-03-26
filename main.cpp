#include <iostream>
#include <omp.h>
#include "vec3.h"
#include "film.h"
#include "ray.h"
#include "shape.h"
#include "camera.h"
#include "primitive.h"
#include "accel.h"
#include "aabb.h"
#include "objloader.h"
#include "light.h"
#include "scene.h"
#include "filter.h"
#include "sampler.h"
#include "material.h"
#include "integrator.h"

int main() {
    Filter* filter = new BoxFilter(Vec2(1));
    Film film(512, 512, std::unique_ptr<Filter>(filter), "output.ppm");
    PinholeCamera cam(Vec3(0, 2, -5), Vec3(0, 0, 1), 1.0f);
    UniformSampler sampler;

    std::vector<std::shared_ptr<Primitive>> prims;
    loadObj(prims, "teapot.obj", Vec3(), 1.0f, std::shared_ptr<Material>(new Lambert(RGB(0.8f))));
    std::vector<std::shared_ptr<Light>> lights;

    Scene scene(prims, lights);
    Integrator* integrator = new PathTrace(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler), 10, 10);

    integrator->render(scene);
}
