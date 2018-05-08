#ifndef INTEGRATOR_H
#define INTEGRATOR_H
#include <omp.h>
#include <memory>
#include "camera.h"
#include "film.h"
#include "sampler.h"
#include "timer.h"
#include "util.h"
class Integrator {
    public:
        std::shared_ptr<Camera> cam;
        std::shared_ptr<Sampler> sampler;

        Integrator(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : cam(_cam), sampler(_sampler) {};

        virtual void render(const Scene& scene) const = 0;
        virtual void compute(const Scene& scene) const = 0;
};


class NormalRenderer : public Integrator {
    public:
        NormalRenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : Integrator(_cam, _sampler) {};

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        cam->film->setPixel(i, j, w*(res.hitNormal + 1.0f)/2.0f);
                    }
                    else {
                        cam->film->setPixel(i, j, w*RGB(0.0f));
                    }
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        cam->film->addSample(i, j, w*(res.hitNormal + 1.0f)/2.0f);
                    }
                    else {
                        cam->film->addSample(i, j, w*RGB(0.0f));
                    }
                }
            }
        };
};


class DotRenderer : public Integrator {
    public:
        DotRenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : Integrator(_cam, _sampler) {};

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        float d = dot(-ray.direction, res.hitNormal);
                        std::cout << d << std::endl;
                        cam->film->setPixel(i, j, w*(std::max(dot(-ray.direction, res.hitNormal), 0.0f)));
                    }
                    else {
                        cam->film->setPixel(i, j, w*RGB(0.0f));
                    }
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {};
};


class BRDFRenderer : public Integrator {
    public:
        BRDFRenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : Integrator(_cam, _sampler) {};

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;
                        Vec3 wo = -ray.direction;
                        Vec3 n = res.hitNormal;
                        Vec3 s = res.dpdu;
                        Vec3 t = normalize(cross(s, n));
                        Vec3 wo_local = worldToLocal(wo, n, s, t);
                        Vec3 wi_local;
                        float brdf_pdf;
                        RGB brdf_f = hitMaterial->sample(wo_local, wi_local, *sampler, brdf_pdf);
                        Vec3 wi = localToWorld(wi_local, n, s, t);
                        cam->film->setPixel(i, j, w*(wi + 1.0f)/2.0f);
                    }
                    else {
                        cam->film->setPixel(i, j, w*RGB(0.0f));
                    }
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) {};
};


class AORenderer : public Integrator {
    public:
        AORenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : Integrator(_cam, _sampler) {};

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;
                        Vec3 wo = -ray.direction;
                        Vec3 n = res.hitNormal;
                        Vec3 s = res.dpdu;
                        Vec3 t = normalize(cross(s, n));
                        Vec3 wo_local = worldToLocal(wo, n, s, t);
                        Vec3 wi_local;
                        float brdf_pdf;
                        int hit_count = 0;
                        for(int k = 0; k < 100; k++) {
                            RGB brdf_f = hitMaterial->sample(wo_local, wi_local, *sampler, brdf_pdf);
                            Vec3 wi = localToWorld(wi, n, s, t);
                            Ray nextRay(res.hitPos, wi);
                            Hit res2;
                            if(scene.intersect(nextRay, res2))
                                hit_count++;
                        }
                        cam->film->setPixel(i, j, hit_count/100.0f*RGB(1.0f));
                    }
                    else {
                        cam->film->setPixel(i, j, w*RGB(0.0f));
                    }
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {};
};


class WireframeRenderer : public Integrator {
    public:
        WireframeRenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler) : Integrator(_cam, _sampler) {};

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Hit res;
                    if(scene.intersect(ray, res)) {
                        RGB col(res.uv.x, res.uv.y, 0.0f);
                        cam->film->setPixel(i, j, w*col);
                    }
                    else {
                        cam->film->setPixel(i, j, w*RGB(0.0f));
                    }
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {};
};


class PathTraceDepthRenderer : public Integrator {
    public:
        int maxDepth;

        PathTraceDepthRenderer(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler, int _maxDepth) : Integrator(_cam, _sampler), maxDepth(_maxDepth) {};

        RGB Li(const Ray& ray, const Scene& scene, int depth = 0, float roulette = 1.0f) const {
            if(depth > maxDepth)
                return RGB(1.0f);

            Hit res;
            RGB col;
            if(scene.intersect(ray, res)) {
                //マテリアル
                const std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;
                //BRDFの計算と方向のサンプリング
                const Vec3 wo = -ray.direction;
                const Vec3 n = res.hitNormal;
                const Vec3 s = res.dpdu;
                const Vec3 t = normalize(cross(s, n));
                Vec3 wo_local = worldToLocal(wo, n, s, t);
                Vec3 wi_local;
                float brdf_pdf;
                const RGB brdf_f = hitMaterial->sample(wo_local, wi_local, *sampler, brdf_pdf);
                Vec3 wi = localToWorld(wi, n, s, t);

                Ray nextRay(res.hitPos, wi);
                return Li(nextRay, scene, depth + 1, roulette);
            }
            else {
                return RGB((float)depth/maxDepth);
            }
        };

        void render(const Scene& scene) const {
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float u = (2.0*i - cam->film->width)/cam->film->width;
                    float v = -(2.0*j - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    RGB col = Li(ray, scene);
                    cam->film->setPixel(i, j, w*col);
                }
            }
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {};
};


class PathTrace : public Integrator {
    public:
        int pixelSamples;
        int maxDepth;

        PathTrace(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler, int _pixelSamples, int _maxDepth) : Integrator(_cam, _sampler), pixelSamples(_pixelSamples), maxDepth(_maxDepth) {};

        RGB Li(const Ray& ray, const Scene& scene, int depth = 0, float roulette = 1.0f) const {
            //ロシアンルーレット
            if(depth > 10) {
                if(sampler->getNext() < roulette) {
                    return RGB(0.0f);
                }
                roulette *= 0.9f;
            }

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
                const std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;

                //ローカル座標系の構築
                const Vec3 wo = -ray.direction;
                const Vec3 n = res.hitNormal;
                const Vec3 s = res.dpdu;
                const Vec3 t = normalize(cross(s, n));
                const Vec3 wo_local = worldToLocal(wo, n, s, t);


                //BRDFの計算と方向のサンプリング
                Vec3 wi_local;
                float brdf_pdf;
                const RGB brdf_f = hitMaterial->sample(wo_local, wi_local, *sampler, brdf_pdf);
                //サンプリングされた方向をワールド座標系に戻す
                Vec3 wi = localToWorld(wi_local, n, s, t);
                //もしbrdfが真っ黒だったらterminate
                //if(brdf_f == RGB(0)) return RGB(0);


                //コサイン項
                const float cos_term = std::abs(wi_local.y);


                //係数
                RGB k = 1.0f/(roulette*brdf_pdf) * cos_term * brdf_f;
                if(k.x < 0.0f || k.y < 0.0f || k.z < 0.0f) {
                    std::cout << "minus k detected" << std::endl;
                    std::cout << "wo: " << wo << std::endl;
                    std::cout << "wo_local: " << wo_local << std::endl;
                    std::cout << "n: " << n << std::endl;
                    std::cout << "wi: " << wi << std::endl;
                    std::cout << "wi_local: " << wi_local << std::endl;
                    std::cout << "cos_term: " << cos_term << std::endl;
                    std::cout << "brdf_f:" << brdf_f << std::endl;
                    std::cout << "k: " << k << std::endl;
                    k = -k;
                }
                if(isnan(k) || isinf(k)) {
                    std::cout << "inf or nan k detected" << std::endl;
                    return RGB(0);
                }

                //レンダリング方程式の計算
                Ray nextRay(res.hitPos, wi);
                col += k * Li(nextRay, scene, depth + 1, roulette);
            }
            else {
                col = scene.sky->getSky(ray);
            }
            return col;
        };


        void render(const Scene& scene) const {
            Timer timer;
            timer.start();
            for(int k = 0; k < pixelSamples; k++) {
                #pragma omp parallel for schedule(dynamic, 1)
                for(int i = 0; i < cam->film->width; i++) {
                    for(int j = 0; j < cam->film->height; j++) {
                        float rx = sampler->getNext();
                        float ry = sampler->getNext();
                        float px = i + rx;
                        float py = j + ry;
                        float u = (2.0*(i + rx) - cam->film->width)/cam->film->height;
                        float v = -(2.0*(j + ry) - cam->film->height)/cam->film->height;
                        float w;
                        Ray ray = cam->getRay(u, v, w, *sampler);
                        RGB col = Li(ray, scene);
                        cam->film->addSample(i, j, w*col);
                    }
                }
                std::cout << progressbar(k, pixelSamples) << " " << percentage(k, pixelSamples) << '\r' << std::flush;
            }
            timer.stop("Rendering Finished");
            cam->film->divide(pixelSamples);
            cam->film->gamma_correction();
            cam->film->ppm_output();
        };
        void compute(const Scene& scene) const {
            #pragma omp parallel for schedule(dynamic, 1)
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float rx = sampler->getNext();
                    float ry = sampler->getNext();
                    float px = i + rx;
                    float py = j + ry;
                    float u = (2.0*(i + rx) - cam->film->width)/cam->film->height;
                    float v = -(2.0*(j + ry) - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    RGB col = Li(ray, scene);
                    cam->film->addSample(i, j, w*col);
                }
            }
        };
};


class PathTraceExplicit : public Integrator {
    public:
        int pixelSamples;
        int maxDepth;

        PathTraceExplicit(std::shared_ptr<Camera> _cam, std::shared_ptr<Sampler> _sampler, int _pixelSamples, int _maxDepth) : Integrator(_cam, _sampler), pixelSamples(_pixelSamples), maxDepth(_maxDepth) {};

        RGB Li(const Ray& ray, const Scene& scene, Vec3& hit_le, int depth = 0, float roulette = 1.0f) const {
            //ロシアンルーレット
            if(depth > 10) {
                if(sampler->getNext() < roulette) {
                    return RGB(0.0f);
                }
                roulette *= 0.9f;
            }

            if(depth > maxDepth)
                return RGB(0.0f);

            Hit res;
            RGB col;
            if(scene.intersect(ray, res)) {
                //光源に当たった場合
                if(res.hitPrimitive->areaLight != nullptr) {
                    //直接光源に当たった場合
                    if(depth == 0) {
                        hit_le = res.hitPrimitive->areaLight->Le(res);
                    }
                    return RGB(0.0f);
                }

                //マテリアル
                const std::shared_ptr<Material> hitMaterial = res.hitPrimitive->material;
                //ローカル座標系の構築
                const Vec3 wo = -ray.direction;
                const Vec3 n = res.hitNormal;
                const Vec3 s = res.dpdu;
                const Vec3 t = normalize(cross(s, n));
                const Vec3 wo_local = worldToLocal(wo, n, s, t);


                //各光源からの寄与を計算
                //DiffuseあるいはGlossyの場合のみに寄与を計算する
                if(hitMaterial->type == MATERIAL_TYPE::DIFFUSE || hitMaterial->type == MATERIAL_TYPE::GLOSSY) {
                    for(const std::shared_ptr<Light> light : scene.lights) {
                        //光源上で点をサンプリング
                        float light_pdf;
                        Vec3 wi_light;
                        const RGB le = light->sample(res, *sampler, wi_light, light_pdf);
                        const Vec3 wi_light_local = worldToLocal(wi_light, n, s, t);

                        //光源に向かうシャドウレイを生成
                        Ray shadowRay(res.hitPos, wi_light);
                        Hit shadow_res;
                        //AreaLight
                        if(light->type == LIGHT_TYPE::AREA) {
                        //シャドウレイが物体に当たったとき、それがサンプリング生成元の光源だった場合は寄与を蓄積
                            if(scene.intersect(shadowRay, shadow_res)) {
                                if(shadow_res.hitPrimitive->areaLight == light) {
                                    col += hitMaterial->f(wo_local, wi_light_local) * le/light_pdf * std::max(wi_light_local.y, 0.0f);
                                }
                                else {
                                    //std::cout << "shadow ray missed" << std::endl;
                                }
                            }
                        }
                        //PointLight
                        else if(light->type == LIGHT_TYPE::POINT) {
                            if(!scene.intersect(shadowRay, shadow_res))
                                col += hitMaterial->f(wo_local, wi_light_local) * le/light_pdf * std::max(wi_light_local.y, 0.0f);
                        }
                        //DirectionalLight
                        else if(light->type == LIGHT_TYPE::DIRECTIONAL) {
                            if(!scene.intersect(shadowRay, shadow_res))
                                col += hitMaterial->f(wo_local, wi_light_local) * le/light_pdf * std::max(wi_light_local.y, 0.0f);
                        }
                    }
                }

                /*
                //もし光源に当たったら放射輝度を蓄積
                if(res.hitPrimitive->areaLight != nullptr) {
                    col += res.hitPrimitive->areaLight->Le(res) * std::max(wo_local.y, 0.0f);
                }
                */


                //BRDFの計算と方向のサンプリング
                Vec3 wi_local;
                float brdf_pdf;
                const RGB brdf_f = hitMaterial->sample(wo_local, wi_local, *sampler, brdf_pdf);
                //サンプリングされた方向をワールド座標系に戻す
                Vec3 wi = localToWorld(wi_local, n, s, t);
                //もしbrdfが真っ黒だったらterminate
                //if(brdf_f == RGB(0)) return RGB(0);


                //コサイン項
                const float cos_term = std::abs(wi_local.y);


                //係数
                RGB k = 1.0f/(roulette*brdf_pdf) * cos_term * brdf_f;
                if(k.x < 0.0f || k.y < 0.0f || k.z < 0.0f) {
                    std::cout << "minus k detected" << std::endl;
                    std::cout << "wo: " << wo << std::endl;
                    std::cout << "wo_local: " << wo_local << std::endl;
                    std::cout << "n: " << n << std::endl;
                    std::cout << "wi: " << wi << std::endl;
                    std::cout << "wi_local: " << wi_local << std::endl;
                    std::cout << "cos_term: " << cos_term << std::endl;
                    std::cout << "brdf_f:" << brdf_f << std::endl;
                    std::cout << "k: " << k << std::endl;
                    k = -k;
                }
                if(isnan(k) || isinf(k)) {
                    std::cout << "inf or nan k detected" << std::endl;
                    return RGB(0);
                }

                //レンダリング方程式の計算
                Ray nextRay(res.hitPos, wi);
                col += k * Li(nextRay, scene, hit_le, depth + 1, roulette);
            }
            else {
                col = scene.sky->getSky(ray);
            }
            return col;
        };


        void render(const Scene& scene) const {
            Timer timer;
            timer.start();
            for(int k = 0; k < pixelSamples; k++) {
                #pragma omp parallel for schedule(dynamic, 1)
                for(int i = 0; i < cam->film->width; i++) {
                    for(int j = 0; j < cam->film->height; j++) {
                        float rx = sampler->getNext();
                        float ry = sampler->getNext();
                        float px = i + rx;
                        float py = j + ry;
                        float u = (2.0*(i + rx) - cam->film->width)/cam->film->height;
                        float v = -(2.0*(j + ry) - cam->film->height)/cam->film->height;
                        float w;
                        Ray ray = cam->getRay(u, v, w, *sampler);
                        Vec3 hit_le;
                        RGB col = Li(ray, scene, hit_le);
                        if(!nonzero(hit_le)) {
                            cam->film->addSample(i, j, w*col);
                        }
                        else {
                            cam->film->addSample(i, j, w*hit_le);
                        };
                    }
                }
                std::cout << progressbar(k, pixelSamples) << " " << percentage(k, pixelSamples) << '\r' << std::flush;
            }
            timer.stop("Rendering Finished");
            cam->film->divide(pixelSamples);
            cam->film->gamma_correction();
            cam->film->ppm_output();
        };


        void compute(const Scene& scene) const {
            #pragma omp parallel for schedule(dynamic, 1)
            for(int i = 0; i < cam->film->width; i++) {
                for(int j = 0; j < cam->film->height; j++) {
                    float rx = sampler->getNext();
                    float ry = sampler->getNext();
                    float px = i + rx;
                    float py = j + ry;
                    float u = (2.0*(i + rx) - cam->film->width)/cam->film->height;
                    float v = -(2.0*(j + ry) - cam->film->height)/cam->film->height;
                    float w;
                    Ray ray = cam->getRay(u, v, w, *sampler);
                    Vec3 hit_le;
                    RGB col = Li(ray, scene, hit_le);
                    if(!nonzero(hit_le)) {
                        cam->film->addSample(i, j, w*col);
                    }
                    else {
                        cam->film->addSample(i, j, w*hit_le);
                    }
                }
            }
        };
};
#endif
