#ifndef FILM_H
#define FILM_H
#include <iostream>
#include <fstream>
#include <string>
#include "util.h"
#include "vec3.h"
class Film {
    public:
        int width;
        int height;
        RGB* data;


        Film(int _width, int _height) : width(_width), height(_height) {
            data = new RGB[width*height];
        };
        ~Film() {
            delete[] data;
        };


        RGB getPixel(int i, int j) const {
            return data[i + width*j];
        };
        void setPixel(int i, int j, const RGB& c) {
            data[i + width*j] = c;
        };


        void ppm_output(const std::string& filename) const {
            std::ofstream file(filename);
            file << "P3" << std::endl;
            file << width << " " << height << std::endl;
            file << 255 << std::endl;
            for(int j = 0; j < height; j++) {
                for(int i = 0; i < width; i++) {
                    RGB c = getPixel(i, j);
                    file << (int)(255*c.x) << " " << (int)(255*c.y) << " " << (int)(255*c.z) << std::endl;
                }
            }
            file.close();
        };
};
#endif
