#ifndef SKY_H
#define SKY_H
#include <string>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include "vec3.h"
#include "ray.h"
class Sky {
    public:
        Sky() {};
        virtual RGB getSky(const Ray& ray) const = 0;
};


class IBL : public Sky {
    public:
        int width;
        int height;
        float *HDRI;

        IBL(const std::string& filename) {
            int n;
            HDRI = stbi_loadf(filename.c_str(), &width, &height, &n, 0);
        };
        ~IBL() {
            stbi_image_free(HDRI);
        };

        RGB getSky(const Ray& ray) const {
            float phi = std::atan2(ray.direction.z, ray.direction.x);
            if(phi < 0) phi += 2*M_PI;
            float theta = std::atan(ray.direction.y/std::sqrt(ray.direction.x*ray.direction.x + ray.direction.z*ray.direction.z)) + M_PI/2;

            float u = phi/(2.0*M_PI);
            float v = 1.0f - theta/M_PI;

            int w = (int)(u * width);
            int h = (int)(v * height);
            int adr = 3*w + 3*width*h;
            return RGB(HDRI[adr], HDRI[adr+1], HDRI[adr+2]);
        };
};
#endif
