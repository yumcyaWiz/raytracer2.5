#include "cpptoml.h"
#include <gperftools/profiler.h>

#include <iostream>
#include <map>
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
#include "light.h"
#include "objloader.h"
#include "scene.h"
#include "filter.h"
#include "sampler.h"
#include "material.h"
#include "integrator.h"
#include "sky.h"
#include "rtoutput.h"


int main(int argc, char** argv) {
    //ファイルパスの読み込み ./a.out -i scene.toml  のように読み込む
    std::string filepath;
    int opt;
    while((opt = getopt(argc, argv, "i:on")) != -1) {
        switch(opt) {
            case 'i':
                filepath = optarg;
                break;
        }
    }



    //tomlの読み込み
    auto toml = cpptoml::parse_file(filepath);



    //film
    auto film_toml = toml->get_table("film");
    if(!film_toml) { std::cerr << "[film] missing" << std::endl; std::exit(1); }
    auto resolution = *film_toml->get_array_of<int64_t>("resolution");
    Filter* filter = new GaussianFilter(Vec2(1), 1.0f);
    std::shared_ptr<Film> film = std::make_shared<Film>(resolution[0], resolution[1], std::unique_ptr<Filter>(filter));
    std::cout << "film loaded" << std::endl;



    //sky
    auto sky = toml->get_table("sky");
    auto sky_type = *sky->get_as<std::string>("type");
    Sky* sky_ptr;
    if(sky_type == "ibl") {
        auto path = *sky->get_as<std::string>("path");
        auto theta_offset = *sky->get_as<double>("theta-offset");
        auto phi_offset = *sky->get_as<double>("phi-offset");
        sky_ptr = new IBL(path, phi_offset, theta_offset);
    }
    else if(sky_type == "test") {
        sky_ptr = new TestSky();
    }
    else if(sky_type == "uniform") {
        auto color = *sky->get_array_of<double>("color");
        sky_ptr = new UniformSky(Vec3(color[0], color[1], color[2]));
    }
    else if(sky_type == "simple") {
        sky_ptr = new SimpleSky();
    }
    std::cout << "sky loaded" << std::endl;



    //camera
    auto camera = toml->get_table("camera");
    auto camera_type = *camera->get_as<std::string>("type");
    auto camera_transform = camera->get_table("transform");
    auto camera_transform_type = *camera_transform->get_as<std::string>("type");
    auto camera_transform_origin = *camera_transform->get_array_of<double>("origin");
    auto camera_transform_target = *camera_transform->get_array_of<double>("target");
    Vec3 camPos(camera_transform_origin[0], camera_transform_origin[1], camera_transform_origin[2]);
    Vec3 camTarget(camera_transform_target[0], camera_transform_target[1], camera_transform_target[2]);
    Vec3 camForward = normalize(camTarget - camPos);
    std::shared_ptr<Camera> cam;
    if(camera_type == "ideal-pinhole") {
        auto fov = *camera->get_as<double>("fov");
        cam = std::make_shared<PinholeCamera>(camPos, camForward, film, toRad(fov));
    }
    else if(camera_type == "full-degree") {
        cam = std::make_shared<FullDegreeCamera>(camPos, camForward, film); 
    }
    else if(camera_type == "thin-lens") {
        auto lensDistance = *camera->get_as<double>("lens-distance");
        auto focusPoint = *camera->get_array_of<double>("focus-point");
        auto fnumber = *camera->get_as<double>("f-number");
        cam = std::make_shared<ThinLensCamera>(camPos, camForward, film, lensDistance, Vec3(focusPoint[0], focusPoint[1], focusPoint[2]), fnumber);
    }
    else if(camera_type == "ods") {
        auto IPD = *camera->get_as<double>("ipd");
        if(resolution[0] != resolution[1]) {
            std::cerr << "Invalid Resolution" << std::endl;
            std::exit(1);
        }
        cam = std::make_shared<ODSCamera>(camPos, camForward, film, IPD);
    }
    else {
        std::cerr << "invalid camera type" << std::endl;
        std::exit(1);
    }
    std::cout << "camera loaded" << std::endl;



    //materials
    std::map<std::string, std::shared_ptr<Material>> material_map;
    auto materials = toml->get_table_array("material");
    for(const auto& material : *materials) {
        auto name = *material->get_as<std::string>("name");
        auto type = *material->get_as<std::string>("type");
        Material* mat = nullptr;
        if(type == "lambert") {
            auto albedo = *material->get_array_of<double>("albedo");
            Vec3 reflectance(albedo[0], albedo[1], albedo[2]);
            mat = new Lambert(reflectance);
        }
        else if(type == "mirror") {
            auto albedo = *material->get_as<double>("albedo");
            mat = new Mirror(albedo);
        }
        else if(type == "phong") {
            auto albedo = *material->get_array_of<double>("albedo");
            auto kd = *material->get_as<double>("kd");
            auto alpha = *material->get_as<double>("alpha");
            Vec3 reflectance(albedo[0], albedo[1], albedo[2]);
            mat = new Phong(reflectance, kd, alpha);
        }
        else if(type == "glass") {
            auto ior = *material->get_as<double>("ior");
            mat = new Glass(ior);
        }
        material_map.insert(std::make_pair(name, std::shared_ptr<Material>(mat)));
    }
    std::cout << "material loaded" << std::endl;



    //meshes
    struct ShapeData {
        std::string type;
        std::string path;
        float radius;

        ShapeData() {};
        ShapeData(const std::string& _type, const std::string& _path, float _radius) : type(_type), path(_path), radius(_radius) {};
    };
    std::map<std::string, ShapeData> mesh_map;
    auto meshes = toml->get_table_array("mesh");
    for(const auto& mesh : *meshes) {
        auto name = *mesh->get_as<std::string>("name");
        auto type = *mesh->get_as<std::string>("type");
        ShapeData shapedata;
        if(type == "sphere") {
            auto radius = *mesh->get_as<double>("radius");
            shapedata = ShapeData(type, "", radius);
        }
        else if(type == "obj") {
            std::string path = *mesh->get_as<std::string>("path");
            shapedata = ShapeData(type, path, 0.0f);
        }
        mesh_map.insert(std::make_pair(name, shapedata));
    }
    std::cout << "mesh loaded" << std::endl;



    //objects
    auto objects = toml->get_table_array("object");
    //プリミティブの配列
    std::vector<std::shared_ptr<Primitive>> prims;
    //ライトの配列
    std::vector<std::shared_ptr<Light>> lights;
    //ShapeとPrimitiveの連想配列(lightの読み込みで名前で参照するときに使用する)
    std::map<std::string, std::shared_ptr<Shape>> shape_map;
    std::map<std::string, std::shared_ptr<Primitive>> prim_map;
    for(const auto& object : *objects) {
        std::string name = *object->get_as<std::string>("name");
        std::string mesh = *object->get_as<std::string>("mesh");
        std::string material = *object->get_as<std::string>("material");
        auto transforms = object->get_table_array("transform");
        Vec3 center;
        Vec3 scale(1, 1, 1);
        for(const auto& transform : *transforms) {
            std::string transform_type = *transform->get_as<std::string>("type");
            if(transform_type == "translate") {
                auto vector = *transform->get_array_of<double>("vector");
                center = Vec3(vector[0], vector[1], vector[2]);
            }
            else if(transform_type == "scale") {
                auto scale_v = *transform->get_array_of<double>("vector");
                scale = Vec3(scale_v[0], scale_v[1], scale_v[2]);
            }
        }
        
        ShapeData shapedata = mesh_map.at(mesh);
        std::shared_ptr<Material> mat = material_map.at(material);
        if(shapedata.type == "sphere") {
            std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(new Sphere(center, shapedata.radius));
            std::shared_ptr<Primitive> prim = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, nullptr, shape));
            prims.push_back(prim);
            shape_map.insert(std::make_pair(name, shape));
            prim_map.insert(std::make_pair(name, prim));
        }
        else if(shapedata.type == "obj") {
            loadObj(prims, lights, shapedata.path, center, scale, mat, name, prim_map, shape_map);
        }
    }
    std::cout << "objects loaded" << std::endl;



    //lights
    auto light_toml = toml->get_table_array("light");
    if(light_toml) {
        for(const auto& light : *light_toml) {
            auto light_emission = *light->get_array_of<double>("emission");
            Vec3 emission(light_emission[0], light_emission[1], light_emission[2]);
            auto light_type = *light->get_as<std::string>("type");

            std::shared_ptr<Light> lightPtr;
            if(light_type == "point") {
                auto lightPos = *light->get_array_of<double>("light-pos");
                Vec3 pos(lightPos[0], lightPos[1], lightPos[2]);
                lightPtr = std::shared_ptr<Light>(new PointLight(pos, emission));
            }
            else if(light_type == "directional") {
                auto direction = *light->get_array_of<double>("direction");
                Vec3 dir = normalize(Vec3(direction[0], direction[1], direction[2]));
                lightPtr = std::shared_ptr<Light>(new DirectionalLight(dir, emission));
            }
            else if(light_type == "area") {
                auto object = *light->get_as<std::string>("object");
                std::shared_ptr<Shape> shape = shape_map.at(object);
                std::shared_ptr<Primitive> prim = prim_map.at(object);
                lightPtr = std::shared_ptr<Light>(new AreaLight(shape, emission));
                prim->areaLight = lightPtr;
            }
            lights.push_back(lightPtr);
        }
    }
    std::cout << "lights loaded" << std::endl;


    //sampler
    std::shared_ptr<Sampler> sampler;
    auto sampler_toml = toml->get_table("sampler");
    auto sampler_type = *sampler_toml->get_as<std::string>("type");
    if(sampler_type == "mt") {
        sampler = std::shared_ptr<Sampler>(new UniformSampler(RNG_TYPE::MT));
    }
    else {
        sampler = std::shared_ptr<Sampler>(new UniformSampler(RNG_TYPE::MINSTD));
    }


    //シーンの初期化
    Scene scene(prims, lights, std::shared_ptr<Sky>(sky_ptr));



    //renderer
    auto renderer = toml->get_table("renderer");
    if(!renderer) { std::cerr << "[renderer] missing" << std::endl; std::exit(1); }
    int samples = *renderer->get_as<int>("samples");
    int depth_limit = *renderer->get_as<int>("depth-limit");
    std::string integrator = *renderer->get_as<std::string>("integrator");
    bool renderer_show = *renderer->get_as<bool>("show");
    bool renderer_profile = *renderer->get_as<bool>("profile");

    Integrator* integ;
    if(integrator == "pt") {
        integ = new PathTrace(cam, sampler, samples, depth_limit);
    }
    else if(integrator == "normal") {
        integ = new NormalRenderer(cam, sampler);
    }
    else if(integrator == "depth") {
      integ = new DepthRenderer(cam, sampler);
    }
    else if(integrator == "albedo") {
      integ = new AlbedoRenderer(cam, sampler);
    }
    else if(integrator == "dot") {
        integ = new DotRenderer(cam, sampler);
    }
    else if(integrator == "wireframe") {
        integ = new WireframeRenderer(cam, sampler);
    }
    else if(integrator == "pt-explicit") {
        integ = new PathTraceExplicit(cam, sampler, samples, depth_limit);
    }
    else {
        integ = new PathTrace(cam, sampler, 10, 100);
    }


    if(renderer_show) {
        //openGLで表示
        drawgl(argc, argv, integ, scene);
    }
    else {
        if(renderer_profile) {
            ProfilerStart("profiler.prof");
            integ->render(scene);
            ProfilerStop();
        }
        else {
            integ->render(scene);
        }
    }
    return 0;
}
