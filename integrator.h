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


class PathTrace : public Integrator {
    public:
        int pixelSamples;
        int maxDepth;

        PathTrace(std::shared_ptr<Camera> _cam, std::shared_ptr<Film> _film, std::shared_ptr<Sampler> _sampler, int _pixelSamples, int _maxDepth) : Integrator(_cam, _film, _sampler), pixelSamples(_pixelSamples), maxDepth(_maxDepth) {};

        RGB Li(const Ray& ray, const Scene& scene, int depth = 0) const {
            if(depth > maxDepth)
                return RGB(0.0f);

            Hit res;
            RGB col;
            if(scene.intersect(ray, res)) {
                //もし光源に当たったら放射輝度を蓄積
                if(res.hitPrimitive->areaLight != nullptr) {
                    col += res.hitPrimitive->areaLight->Le(res);
                }
                //マテリアル
                std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;
                //BRDFの計算と方向のサンプリング
                Vec3 n = res.hitNormal;
                Vec3 s = normalize(res.dpdu);
                Vec3 t = normalize(cross(s, n));
                Vec3 wi;
                float brdf_pdf;
                RGB brdf_f = hitMaterial->sample(-ray.direction, wi, n, s, t, sampler->getNext2D(), brdf_pdf);

                return brdf_f * dot(res.hitNormal, wi)/brdf_pdf * Li(Ray(res.hitPos, wi), scene, depth + 1);
            }
            else {
                return RGB(0.5f);
            }
        };

        void render(const Scene& scene) const {
            for(int k = 0; k < pixelSamples; k++) {
                for(int i = 0; i < film->width; i++) {
                    for(int j = 0; j < film->height; j++) {
                        float rx = sampler->getNext();
                        float ry = sampler->getNext();
                        float px = i + rx;
                        float py = j + ry;
                        float u = (2.0*i - film->width + rx)/film->width;
                        float v = -(2.0*j - film->height + ry)/film->height;
                        Ray ray = cam->getRay(u, v);
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
