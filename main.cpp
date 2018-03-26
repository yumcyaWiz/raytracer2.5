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
    PinholeCamera cam(Vec3(0, 5, -10), Vec3(0, 0, 1), 1.0f);
    UniformSampler sampler;
    
    std::shared_ptr<Material> mat = std::shared_ptr<Material>(new Lambert(RGB(0.8f)));
    std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(new Sphere(Vec3(0, -1000, 0), 1000.0f));
    std::shared_ptr<Primitive> prim = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape));

    std::vector<std::shared_ptr<Primitive>> prims;
    prims.push_back(prim);
    loadObj(prims, "dragon.obj", Vec3(), 1.0f, std::shared_ptr<Material>(new Lambert(RGB(0.0f, 1.0f, 0.0f))));

    std::vector<std::shared_ptr<Light>> lights;

    Scene scene(prims, lights);
    Integrator* integrator = new PathTrace(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler), 10, 10);

    integrator->render(scene);
}
