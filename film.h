#ifndef FILM_H
#define FILM_H
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "util.h"
#include "vec3.h"
#include "filter.h"
class Film {
    public:
        struct Pixel {
            RGB color_sum;
            float filter_sum;

            Pixel() : color_sum(RGB(0)), filter_sum(0.0f) {};
            Pixel(const RGB& _color_sum, float _filter_sum) : color_sum(_color_sum), filter_sum(_filter_sum) {};
        };


        int width;
        int height;
        Pixel* pixels;
        std::unique_ptr<Filter> filter;
        const std::string filename;


        Film(int _width, int _height, std::unique_ptr<Filter> _filter, const std::string& _filename) : width(_width), height(_height), filter(std::move(_filter)), filename(_filename) {
            pixels = new Pixel[width*height];
        };
        ~Film() {
            delete[] pixels;
        };


        RGB getPixel(int i, int j) const {
            return pixels[i + width*j].color_sum;
        };
        void setPixel(int i, int j, const RGB& c) {
            pixels[i + width*j].color_sum = c;
        };
        void setPixel(int i, int j, const Pixel& _pix) {
            pixels[i + width*j] = _pix;
        };
        void addSample(int i, int j, const RGB& c) {
            pixels[i + width*j].color_sum += c;
        };
        void addSampleByFilter(float i, float j, const RGB& c) {
            int pminX = inrangeX(std::ceil(i - filter->radius.x));
            int pmaxX = inrangeX(std::floor(i + filter->radius.x));
            int pminY = inrangeY(std::ceil(j - filter->radius.y));
            int pmaxY = inrangeY(std::floor(j + filter->radius.x));
            for(int ix = pminX; ix <= pmaxX; ix++) {
                for(int iy = pminY; iy <= pmaxY; iy++) {
                    float filt = filter->eval(Vec2(ix - i, iy - j));
                    pixels[ix + width*iy].color_sum += filt*c;
                    pixels[ix + width*iy].filter_sum += filt;
                }
            }
        };


        void divide(int k) {
            for(int i = 0; i < width; i++) {
                for(int j = 0; j < height; j++) {
                    pixels[i + width*j].color_sum /= k;
                    pixels[i + width*j].filter_sum /= k;
                }
            }
        };
        void finalize() {
            for(int i = 0; i < width; i++) {
                for(int j = 0; j < height; j++) {
                    float fsum = pixels[i + width*j].filter_sum;
                    if(fsum > 0)
                        pixels[i + width*j].color_sum /= fsum;
                }
            }
        };


        void ppm_output() const {
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


    private:
        int inrangeX(int x) const {
            if(x < 0) return 0;
            else if(x >= width - 1) return width - 1;
            else return x;
        }
        int inrangeY(int y) const {
            if(y < 0) return 0;
            else if(y >= height -1) return height - 1;
            else return y;
        };
};
#endif
