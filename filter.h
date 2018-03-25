#ifndef FILTER_H
#define FILTER_H
#include <cmath>
#include "vec2.h"
class Filter {
    public:
        Vec2 radius;

        Filter(const Vec2& _radius) : radius(_radius) {};

        virtual float eval(const Vec2& p) const = 0;
};


class BoxFilter : public Filter {
    public:
        BoxFilter(const Vec2& _radius) : Filter(_radius) {};

        float eval(const Vec2& p) const {
            return 1.0f;
        };
};


class TriangleFilter : public Filter {
    public:
        TriangleFilter(const Vec2& _radius) : Filter(_radius) {};

        float eval(const Vec2& p) const {
            return std::max(0.0f, radius.x - std::abs(p.x))*std::max(0.0f, radius.y - std::abs(p.y));
        };
};


class GaussianFilter : public Filter {
    public:
        float alpha;
        float expX, expY;

        GaussianFilter(const Vec2& _radius, float _alpha) : Filter(_radius), alpha(_alpha), expX(std::exp(-alpha*_radius.x*_radius.x)), expY(std::exp(-alpha*_radius.y*_radius.y)) {};

        float eval(const Vec2& p) const {
            return gaussian(p.x, expX)*gaussian(p.y, expY);
        };

    private:
        float gaussian(float x, float expv) const {
            return std::max(std::exp(-alpha*x*x) - expv, 0.0f);
        };
};
#endif
