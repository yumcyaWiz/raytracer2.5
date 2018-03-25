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

int main() {
    Filter* filter = new BoxFilter(Vec2(1));
    Film film(512, 512, std::unique_ptr<Filter>(filter), "output.ppm");
    PinholeCamera cam(Vec3(0, 2, -5), Vec3(0, 0, 1), 1.0f);
    Sphere sphere(Vec3(0), 1.0f);
    UniformSampler sampler;

    std::vector<std::shared_ptr<Primitive>> prims;
    
    loadObj(prims, "teapot.obj", Vec3(), 1.0f);
    BVH bvh = BVH(prims, 4, BVH_PARTITION_TYPE::SAH);

    #pragma omp parallel for schedule(dynamic, 1)
    for(int i = 0; i < film.width; i++) {
        for(int j = 0; j < film.height; j++) {
            float rx = sampler.getNext();
            float ry = sampler.getNext();
            float px = i + rx;
            float py = j + ry;
            float u = (2.0*i - film.width + rx)/film.width;
            float v = -(2.0*j - film.height + ry)/film.height;

            Ray ray = cam.getRay(u, v);
            Hit res;
            if(bvh.intersect(ray, res)) {
                film.addSample(i, j, (res.hitNormal + 1.0f)/2.0f);
            }
            else {
                film.addSample(i, j, RGB(0.0f));
            }
            //std::cout << film.pixels[i + film.width+j].color_sum << std::endl;
            //std::cout << film.pixels[i + film.width+j].filter_sum << std::endl;
        }
        if(omp_get_thread_num() == 0)
            std::cout << progressbar(i, film.width) << " " << percentage(i, film.width) << "\r" << std::flush;
    }
    film.finalize();
    film.ppm_output();
}
