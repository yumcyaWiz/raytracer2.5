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

int main() {
    Film film(512, 512);
    PinholeCamera cam(Vec3(0, 5, -10), Vec3(0, 0, 1), 1.0f);
    Sphere sphere(Vec3(0), 1.0f);

    std::vector<std::shared_ptr<Primitive>> prims;
    
    loadObj(prims, "dragon.obj", Vec3(), 1.0f);
    BVH bvh = BVH(prims, 4, BVH_PARTITION_TYPE::SAH);

    #pragma omp parallel for schedule(dynamic, 1)
    for(int i = 0; i < film.width; i++) {
        for(int j = 0; j < film.height; j++) {
            float u = (2.0*i - film.width)/film.width;
            float v = -(2.0*j - film.height)/film.height;
            Ray ray = cam.getRay(u, v);
            Hit res;
            if(bvh.intersect(ray, res)) {
                film.setPixel(i, j, (res.hitNormal + 1.0f)/2.0f);
            }
            else {
                film.setPixel(i, j, RGB(0));
            }
        }
        if(omp_get_thread_num() == 1)
            std::cout << progressbar(i, film.width) << " " << percentage(i, film.width) << "\r" << std::flush;
    }
    film.ppm_output("output.ppm");
}
