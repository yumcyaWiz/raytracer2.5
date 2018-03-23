#include <iostream>
#include "vec3.h"
#include "film.h"
#include "ray.h"
#include "shape.h"
#include "camera.h"
#include "primitive.h"
#include "accel.h"
#include "aabb.h"

int main() {
    Film film(512, 512);
    PinholeCamera cam(Vec3(0, 0, -3), Vec3(0, 0, 1), 1.0f);
    Sphere sphere(Vec3(0), 1.0f);

    std::vector<std::shared_ptr<Primitive>> prims;
    
    prims.push_back(std::shared_ptr<Primitive>(new GeometricPrimitive(std::shared_ptr<Shape>(&sphere))));
    BVH bvh = BVH(prims, 1, BVH_PARTITION_TYPE::SAH);

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
    }
    film.ppm_output("output.ppm");
}
