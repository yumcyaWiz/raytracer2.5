#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <cstdlib>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#endif

#include "vec2.h"
#include "vec3.h"
#include "shape.h"
#include "primitive.h"


void loadPolygon(const std::vector<std::shared_ptr<Triangle>>& triangles, const std::shared_ptr<Material> _mat, bool mtl, const tinyobj::material_t material, std::vector<std::shared_ptr<Primitive>>& prims, std::vector<std::shared_ptr<Light>>& lights, bool map_insert, const std::string& name, std::map<std::string, std::shared_ptr<Primitive>>& prim_map, std::map<std::string, std::shared_ptr<Shape>>& shape_map) {
    std::shared_ptr<Shape> shape = std::shared_ptr<Shape>(new Polygon(triangles));
    std::shared_ptr<Material> mat;
    std::shared_ptr<Light> light;
    if(mtl) {
        int illum = material.illum;

        //light
        Vec3 ke(material.emission[0], material.emission[1], material.emission[2]);
        if(nonzero(ke)) {
            light = std::shared_ptr<Light>(new AreaLight(shape, ke));
            lights.push_back(light);
        }

        //diffuse
        Vec3 kd(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
        if(illum == 2) {
            mat = std::shared_ptr<Material>(new Lambert(kd));
        }
        //mirror
        else if(illum == 5) {
            Vec3 Ks(material.specular[0], material.specular[1], material.specular[2]);
            mat = std::shared_ptr<Material>(new Mirror(Ks[0]));
        }
        //glass
        else if(illum == 7) {
            mat = std::shared_ptr<Material>(new Glass(1.5f));
        }
        else {
            if(!nonzero(kd)) kd = Vec3(0.9);
            mat = std::shared_ptr<Material>(new Lambert(kd));
        }
    }
    else {
        mat = _mat;
    }

    std::shared_ptr<Primitive> prim = std::shared_ptr<Primitive>(new GeometricPrimitive(mat, light, shape));
    prims.push_back(prim);

    if(map_insert) {
        shape_map.insert(std::make_pair(name, shape));
        prim_map.insert(std::make_pair(name, prim));
    }
}


//objファイルを読み込み、std:shared_ptr<Triangle>の配列を返す
void loadObj(std::vector<std::shared_ptr<Primitive>>& prims, std::vector<std::shared_ptr<Light>>& lights, const std::string& filename, const Vec3& center, const Vec3& scale, std::shared_ptr<Material> _mat, const std::string& name, std::map<std::string, std::shared_ptr<Primitive>>& prim_map, std::map<std::string, std::shared_ptr<Shape>>& shape_map) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
    if(!err.empty())
        std::cerr << err << std::endl;
    if(!ret)
        std::exit(1);

    bool mtl = !materials.empty();

    int face_count = 0;
    int vertex_count = 0;
    for(size_t s = 0; s < shapes.size(); s++) {
        std::cout << "Loading " << shapes[s].name << std::endl;

        //三角形の配列
        std::vector<std::shared_ptr<Triangle>> triangles;
        size_t index_offset = 0;
        int prev_material_id = 0;
        for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            std::vector<Vec3> vertex;
            std::vector<Vec3> normal;
            std::vector<Vec2> uv;
            for(size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
                vertex.push_back(Vec3((float)vx, (float)vy, (float)vz));

                if(idx.normal_index > 0) {
                    tinyobj::real_t nx = attrib.normals[3*idx.normal_index+0];
                    tinyobj::real_t ny = attrib.normals[3*idx.normal_index+1];
                    tinyobj::real_t nz = attrib.normals[3*idx.normal_index+2];
                    normal.push_back(Vec3((float)nx, (float)ny, (float)nz));
                }

                vertex_count++;
            }
            index_offset += fv;

            std::shared_ptr<Triangle> triangle;
            if(normal.size() > 0) {
                triangle = std::shared_ptr<Triangle>(new Triangle(center + scale*vertex[0], center + scale*vertex[1], center + scale*vertex[2], normal[0], normal[1], normal[2]));
            }
            else {
                triangle = std::shared_ptr<Triangle>(new Triangle(center + scale*vertex[0], center + scale*vertex[1], center + scale*vertex[2]));
            }

            triangles.push_back(triangle);
            face_count++;

            //マテリアルの変更を検出
            if(f != 0 && shapes[s].mesh.material_ids[f] != prev_material_id) {
                auto material = materials[shapes[s].mesh.material_ids[f]];
                loadPolygon(triangles, _mat, mtl, material, prims, lights, false, name, prim_map, shape_map);
                triangles = std::vector<std::shared_ptr<Triangle>>();
            }
            prev_material_id = shapes[s].mesh.material_ids[f];
        }

        tinyobj::material_t material;
        if(mtl) {
            material = materials[shapes[s].mesh.material_ids[0]];
            loadPolygon(triangles, _mat, mtl, material, prims, lights, shapes.size() == 1, name, prim_map, shape_map);
        }
        else {
            loadPolygon(triangles, _mat, false, material, prims, lights, shapes.size() == 1, name, prim_map, shape_map);
        }
    }
    std::cout << "total vertex:" << vertex_count << std::endl;
    std::cout << "total face:" << face_count << std::endl;
}


#endif
