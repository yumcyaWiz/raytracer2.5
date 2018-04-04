#ifndef VEC3_H
#define VEC3_H
#include <iostream>
#include <cmath>
class Vec3 {
    public:
        float x;
        float y;
        float z;

        Vec3() { x = y = z = 0; };
        Vec3(float _x) : x(_x), y(_x), z(_x) {};
        Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

        Vec3 operator+(const Vec3& v) const {
            return Vec3(x + v.x, y + v.y, z + v.z);
        };
        Vec3 operator+(float k) const {
            return Vec3(x + k, y + k, z + k);
        };
        Vec3 operator-(const Vec3& v) const {
            return Vec3(x - v.x, y - v.y, z - v.z);
        };
        Vec3 operator-(float k) const {
            return Vec3(x - k, y - k, z - k);
        };
        Vec3 operator*(const Vec3& v) const {
            return Vec3(x * v.x, y * v.y, z * v.z);
        };
        Vec3 operator*(float k) const {
            return Vec3(x * k, y * k, z * k);
        };
        Vec3 operator/(const Vec3& v) const {
            return Vec3(x / v.x, y / v.y, z / v.z);
        };
        Vec3 operator/(float k) const {
            return Vec3(x / k, y / k, z / k);
        };

        void operator+=(const Vec3& v) {
            x += v.x;
            y += v.y;
            z += v.z;
        };
        void operator-=(const Vec3& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
        };
        void operator*=(const Vec3& v) {
            x *= v.x;
            y *= v.y;
            z += v.z;
        };
        void operator/=(const Vec3& v) {
            x /= v.x;
            y /= v.y;
            z /= v.z;
        };
        void operator/=(float k) {
            x /= k;
            y /= k;
            z /= k;
        };

        Vec3 operator-() const {
            return Vec3(-x, -y, -z);
        };

        bool operator==(const Vec3& v) const {
            return x == v.x && y == v.y && z == v.z;
        };

        float operator[](int i) const {
            if(i == 0) return x;
            else if(i == 1) return y;
            else return z;
        };

            

        float length() const {
            return std::sqrt(x*x + y*y + z*z);
        };
        float length2() const {
            return x*x + y*y + z*z;
        };
};
using RGB = Vec3;


inline Vec3 operator+(float k, const Vec3& v) {
    return v + k;
}
inline Vec3 operator-(float k, const Vec3& v) {
    return Vec3(k - v.x, k - v.y, k - v.z);
}
inline Vec3 operator*(float k, const Vec3& v) {
    return v * k;
}
inline Vec3 operator/(float k, const Vec3& v) {
    return Vec3(k / v.x, k / v.y, k / v.z);
}


inline float dot(const Vec3& v1, const Vec3& v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
inline Vec3 cross(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}


inline Vec3 normalize(const Vec3& v) {
    return v/v.length();
}


inline std::ostream& operator<<(std::ostream& stream, const Vec3& v) {
    stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return stream;
}


inline Vec3 max(const Vec3& v1, const Vec3& v2) {
    return Vec3(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z));
}
inline Vec3 min(const Vec3& v1, const Vec3& v2) {
    return Vec3(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z));
}


inline Vec3 omitNA(const Vec3& v) {
    Vec3 ret(v);
    if(std::isnan(v.x) || std::isinf(v.x)) {
        ret.x = 0.0f;
    }
    if(std::isnan(v.y) || std::isinf(v.y)) {
        ret.y = 0.0f;
    }
    if(std::isnan(v.z) || std::isinf(v.z)) {
        ret.z = 0.0f;
    }
    return ret;
}
#endif
