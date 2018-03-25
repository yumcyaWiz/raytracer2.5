#ifndef INTEGRATOR_H
#define INTEGRATOR_H
#include <memory>
#include "camera.h"
#include "film.h"
#include "sampler.h"
class Integrator {
    public:
        std::shared_ptr<Camera> cam;
        std::shared_ptr<Film> film;
        std::shared_ptr<Sampler> sampler;

        Integrator(std::shared_ptr<Camera> _cam, std::shared_ptr<Film> _film, std::shared_ptr<Sampler> _sampler) : cam(_cam), film(_film), sampler(_sampler) {};

        virtual void render(const Scene& scene) const = 0;
};


class DirectIllumination : public Integrator {
    public:
};


class PathTrace : public Integrator {
    public:
        int pixelSamples;
        int maxDepth;

        PathTrace(std::shared_ptr<Camera> _cam, std::shared_ptr<Film> _film, std::shared_ptr<Sampler> _sampler, int _pixelSamples, int _maxDepth) : Integrator(_cam, _film, _sampler), pixelSamples(_pixelSamples), maxDepth(_maxDepth) {};

        RGB Li(const Ray& ray, const Scene& scene) {
            Hit res;
            RGB col;
            if(scene->intersect(ray, res)) {
                if(res.hitPrimitive.areaLight != nullptr) {
                    col += res.hitPrimitive->areaLight->Le(res);
                }
            }
            else {
            }
        };

        void render(const Scene& scene) const {
            for(int k = 0; k < pixelSamples; k++) {
                for(int i = 0; i < film->width; i++) {
                    for(int j = 0; j < film->height; j++) {
                        float rx = sampler.getNext();
                        float ry = sampler.getNext();
                        float px = i + rx;
                        float py = j + ry;
                        float u = (2.0*i - film.width + rx)/film.width;
                        float v = -(2.0*j - film.height + ry)/film.height;
                        Ray ray = cam.getRay(u, v);
                        RGB col = Li(ray, scene);
                        film->addSample(i, j, col);
                    }
                }
            }
            film->divide(pixelSamples);
            film->ppm_output();
        };
};
#endif
