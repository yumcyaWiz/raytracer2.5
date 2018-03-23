#include <iostream>
#include "vec3.h"
#include "film.h"
#include "ray.h"

int main() {
    Film film(512, 512);
    for(int i = 0; i < film.width; i++) {
        for(int j = 0; j < film.height; j++) {
            film.setPixel(i, j, RGB(0, 1, 0));
        }
    }
    film.ppm_output("output.ppm");
}
