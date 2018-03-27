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
#include "sky.h"

int main() {
    Filter* filter = new GaussianFilter(Vec2(1), 1.0f);
    Film film(512, 512, std::unique_ptr<Filter>(filter), "output.ppm");
    PinholeCamera cam(Vec3(0, 5, -10), Vec3(0, 0, 1), 1.0f);
    UniformSampler sampler(RNG_TYPE::MINSTD);

    
    std::shared_ptr<Material> mat = std::shared_ptr<Material>(new Lambert(RGB(0.8f)));
    std::shared_ptr<Material> mat2 = std::shared_ptr<Material>(new Mirror(RGB(0.8f)));
    std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(new Sphere(Vec3(0, -3000, 0), 3000.0f));
    std::shared_ptr<Shape> shape2 = std::shared_ptr<Shape>(new Sphere(Vec3(0.2, 0.8f, -2.5), 0.8f));
    std::shared_ptr<Primitive> prim = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape));
    std::shared_ptr<Primitive> prim2 = std::shared_ptr<Primitive>(new GeometricPrimitive(mat2, nullptr, shape2));

    std::vector<std::shared_ptr<Primitive>> prims;
    prims.push_back(prim);
    prims.push_back(prim2);
    loadObj(prims, "dragon.obj", Vec3(), 1.0f, std::shared_ptr<Material>(new Lambert(RGB(0.8f))));

    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Sky> sky = std::shared_ptr<Sky>(new SimpleSky());

    Scene scene(prims, lights, sky);
    Integrator* integrator = new PathTrace(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler), 10, 100);
    //Integrator* integrator = new PathTraceDepthRenderer(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler), 10);

    integrator->render(scene);
}
