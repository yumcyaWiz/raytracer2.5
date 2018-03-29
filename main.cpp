#include <iostream>
#include <omp.h>
#include <unistd.h>
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


int main(int argc, char** argv) {
    int width = 512;
    int height = 512;
    int samples = 100;
    int opt;
    while((opt = getopt(argc, argv, "w:h:s:on")) != -1) {
        switch(opt) {
            case 'w':
                width = std::stoi(optarg);
                break;
            case 'h':
                height = std::stoi(optarg);
                break;
            case 's':
                samples = std::stoi(optarg);
                break;
        }
    }


    Filter* filter = new GaussianFilter(Vec2(1), 1.0f);
    Film film(width, height, std::unique_ptr<Filter>(filter), "output.ppm");
    
    Vec3 camPos = Vec3(0, 2, -5);
    PinholeCamera cam(camPos, normalize(Vec3(0, 1.0f, -0.422f) - camPos), 1.0f);
    UniformSampler sampler(RNG_TYPE::MT);
    
    std::shared_ptr<Material> mat = std::shared_ptr<Material>(new Lambert(RGB(0.8f)));
    std::shared_ptr<Material> mat2 = std::shared_ptr<Material>(new Mirror(RGB(0.8f)));
    std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(new Sphere(Vec3(0, -3000, 0), 3000.0f));
    std::shared_ptr<Shape> shape2 = std::shared_ptr<Shape>(new Sphere(Vec3(1.0f, 1.0f, -1.0f), 1.0f));
    std::shared_ptr<Shape> shape3 = std::shared_ptr<Shape>(new Sphere(Vec3(-1.0f, 1.0f, -1.0f), 1.0f));
    std::shared_ptr<Shape> shape4 = std::shared_ptr<Shape>(new Sphere(Vec3(0.0f, 1.0f, 0.732f), 1.0f));
    std::shared_ptr<Shape> shape5 = std::shared_ptr<Shape>(new Sphere(Vec3(0.0f, 2.6329f, -0.422f), 1.0f));
    std::shared_ptr<Primitive> prim = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape));
    std::shared_ptr<Primitive> prim2 = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape2));
    std::shared_ptr<Primitive> prim3 = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape3));
    std::shared_ptr<Primitive> prim4 = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape4));
    std::shared_ptr<Primitive> prim5 = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape5));

    std::vector<std::shared_ptr<Primitive>> prims;
    //prims.push_back(prim);
    prims.push_back(prim2);
    prims.push_back(prim3);
    prims.push_back(prim4);
    prims.push_back(prim5);
    loadObj(prims, "plane.obj", Vec3(), 1.0f, mat2);
    //loadObj(prims, "teapot.obj", Vec3(0, 0, 0), 0.15f, mat2);

    std::vector<std::shared_ptr<Light>> lights;
    //std::shared_ptr<Sky> sky = std::shared_ptr<Sky>(new IBL("PaperMill_E_3k.hdr"));
    std::shared_ptr<Sky> sky = std::shared_ptr<Sky>(new TestSky());

    Scene scene(prims, lights, sky);
    Integrator* integrator = new PathTrace(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler), samples, 100);
    //Integrator* integrator = new AORenderer(std::shared_ptr<Camera>(&cam), std::shared_ptr<Film>(&film), std::shared_ptr<Sampler>(&sampler));

    integrator->render(scene);
}
