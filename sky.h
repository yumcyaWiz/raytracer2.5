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


class TestSky : public Sky {
    public:
        TestSky() {};

        RGB getSky(const Ray& ray) const {
            return (ray.direction + 1.0f)/2.0f;
        };
};


class SimpleSky : public Sky {
    public:
        SimpleSky() {};

        RGB getSky(const Ray& ray) const {
            float t = (ray.direction.y + 1.0f)*0.5f;
            return (1.0f - t)*RGB(1.0f) + t*RGB(0.5f, 0.7f, 1.0f);
        };
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
            float theta = std::acos(clamp(ray.direction.y, -1.0f, 1.0f));

            float u = phi/(2.0*M_PI);
            float v = theta/M_PI;

            int w = (int)(u * width);
            int h = (int)(v * height);
            int adr = 3*w + 3*width*h;
            return RGB(HDRI[adr], HDRI[adr+1], HDRI[adr+2]);
        };
};
#endif
