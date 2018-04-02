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


/*
int slashCount(const std::string& str) {
    std::stringstream ss(str);
    std::string buffer;
    int count = 0;
    while(std::getline(ss, buffer, '/')) {
        count++;
    }
    return count - 1;
}
std::vector<std::string> split(const std::string& str, char s) {
    std::stringstream ss(str);
    std::string buffer;
    std::vector<std::string> ret;
    while(std::getline(ss, buffer, s)) {
        ret.push_back(buffer);
    }
    return ret;
}


std::vector<std::string> space_split(const std::string& str) {
    std::vector<std::string> ret;
    std::string buf;
    bool flag = false;
    for(size_t i = 0; i < str.size(); i++) {
        if(str[i] == ' ') {
            if(flag && buf.size() > 0) {
                ret.push_back(buf);
                buf = "";
                flag = false;
            }
        }
        else {
            buf += str[i];
            flag = true;
        }
    }
    if(buf.size() > 0)
        ret.push_back(buf);

    return ret;
}


void loadObj(std::vector<std::shared_ptr<Primitive>>& prims, const std::string& filename, const Vec3& center, float scale, std::shared_ptr<Material> material) {
    struct Face {
        int v1, v2, v3;
        int vt1, vt2, vt3;
        int vn1, vn2, vn3;

        Face(int _v1, int _v2, int _v3) : v1(_v1), v2(_v2), v3(_v3) {};
        Face(int _v1, int _v2, int _v3, int _vt1, int _vt2, int _vt3) : v1(_v1), v2(_v2), v3(_v3), vt1(_vt1), vt2(_vt2), vt3(_vt3) {};
        Face(int _v1, int _v2, int _v3, int _vt1, int _vt2, int _vt3, int _vn1, int _vn2, int _vn3) : v1(_v1), v2(_v2), v3(_v3), vt1(_vt1), vt2(_vt2), vt3(_vt3), vn1(_vn1), vn2(_vn2), vn3(_vn3) {};
    };

    std::vector<Vec3> vertex;
    std::vector<Vec2> uv;
    std::vector<Vec3> normal;
    std::vector<Face> face;

    std::ifstream file(filename);
    if(!file) {
        std::cerr << filename << " doesn't exist" << std::endl;
        std::exit(1);
    }

    //read text by line
    std::string line;
    while(std::getline(file, line)) {
        //separated text in this line
        std::vector<std::string> sepLine;

        //split by space
        sepLine = space_split(line);

        //empty line
        if(sepLine.size() == 0) continue;

        //vertex
        if(sepLine[0] == "v") {
            float x = std::stof(sepLine[1]);
            float y = std::stof(sepLine[2]);
            float z = std::stof(sepLine[3]);
            vertex.push_back(Vec3(x, y, z));
        }
        //texture
        else if(sepLine[0] == "vt") {
            float u = std::stof(sepLine[1]);
            float v = std::stof(sepLine[2]);
            uv.push_back(Vec2(u, v));
        }
        //normal 
        else if(sepLine[0] == "vn") {
            float x = std::stof(sepLine[1]);
            float y = std::stof(sepLine[2]);
            float z = std::stof(sepLine[3]);
            normal.push_back(Vec3(x, y, z));
        }
        //face
        else if(sepLine[0] == "f") {
            int count = slashCount(sepLine[1]);
            if(count == 0) {
                int v1 = std::stoi(sepLine[1]);
                int v2 = std::stoi(sepLine[2]);
                int v3 = std::stoi(sepLine[3]);
                face.push_back(Face(v1, v2, v3));
            }
            else if(count == 1) {
                auto tex1 = split(sepLine[1], '/');
                auto tex2 = split(sepLine[2], '/');
                auto tex3 = split(sepLine[3], '/');
                int v1 = std::stoi(tex1[0]);
                int vt1 = std::stoi(tex1[1]);
                int v2 = std::stoi(tex2[0]);
                int vt2 = std::stoi(tex2[1]);
                int v3 = std::stoi(tex3[0]);
                int vt3 = std::stoi(tex3[1]);
                face.push_back(Face(v1, v2, v3, vt1, vt2, vt3));
            }
            else if(count == 2) {
                auto tex1 = split(sepLine[1], '/');
                auto tex2 = split(sepLine[2], '/');
                auto tex3 = split(sepLine[3], '/');
                int v1 = std::stoi(tex1[0]);
                int vt1 = std::stoi(tex1[1]);
                int vn1 = std::stoi(tex1[2]);
                int v2 = std::stoi(tex2[0]);
                int vt2 = std::stoi(tex2[1]);
                int vn2 = std::stoi(tex2[2]);
                int v3 = std::stoi(tex3[0]);
                int vt3 = std::stoi(tex3[1]);
                int vn3 = std::stoi(tex3[2]);
                face.push_back(Face(v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3));
            }
            else {
                continue;
            }
        }
    }
    file.close();
    std::cout << filename << " loaded" << std::endl;

    //make triangle
    for(const Face& f : face) {
        Vec3 p1 = scale*vertex[f.v1 - 1] + center;
        Vec3 p2 = scale*vertex[f.v2 - 1] + center;
        Vec3 p3 = scale*vertex[f.v3 - 1] + center;
        Primitive* prim = new GeometricPrimitive(material, nullptr, std::shared_ptr<Shape>(new Triangle(p1, p2, p3)));
        prims.push_back(std::shared_ptr<Primitive>(prim));
    }
    std::cout << face.size() << " Triangles added" << std::endl;

    //summary
    std::cout << "Vertex:" << 3*vertex.size() << std::endl;
    std::cout << "Face:" << face.size() << std::endl;
}
*/


void loadObj(std::vector<std::shared_ptr<Primitive>>& prims, const std::string& filename, const Vec3& center, float scale, std::shared_ptr<Material> material) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
    if(!err.empty())
        std::cerr << err << std::endl;
    if(!ret)
        std::exit(1);

    int face_count = 0;
    int vertex_count = 0;
    for(size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
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

            std::shared_ptr<Shape> shape;
            if(normal.size() > 0) {
                shape = std::shared_ptr<Shape>(new Triangle(center + scale*vertex[0], center + scale*vertex[1], center + scale*vertex[2], normal[0], normal[1], normal[2]));
            }
            else {
                shape = std::shared_ptr<Shape>(new Triangle(center + scale*vertex[0], center + scale*vertex[1], center + scale*vertex[2]));
            }

            std::shared_ptr<Primitive> prim(new GeometricPrimitive(material, nullptr, shape));
            prims.push_back(prim);
            face_count++;
        }
    }

    std::cout << "vertex:" << vertex_count << std::endl;
    std::cout << "face:" << face_count << std::endl;
}


#endif
