#ifndef UTIL_H
#define UTIL_H
#include <cmath>
template <typename T>
inline T clamp(T x, T min, T max) {
    if(x < min)
        return min;
    else if(x > max)
        return max;
    else 
        return x;
}


template <typename T>
inline T sign(T x) {
    if(x > 0) return 1;
    else return -1;
}


inline std::string percentage(float x, float max) {
    return std::to_string(x/max*100) + "%";
}
inline std::string progressbar(float x, float max) {
    const int max_count = 40;
    int cur_count = (int)(x/max * max_count);
    std::string str;
    str += "[";
    for(int i = 0; i < cur_count; i++)
        str += "#";
    for(int i = 0; i < (max_count - cur_count - 1); i++)
        str += " ";
    str += "]";
    return str;
}


inline float toRad(float deg) {
    return deg/180.0f * M_PI;
}


//ワールド座標系の方向ベクトルをマテリアル座標系の方向ベクトルに変換する
//w:ワールド座標系方向ベクトル n:法線 s:dpdu t:dpdv
inline Vec3 worldToLocal(const Vec3& w, const Vec3& n, const Vec3& s, const Vec3& t) {
    return Vec3(dot(s, w), dot(n, w), dot(t, w));
}
inline Vec3 localToWorld(const Vec3& w, const Vec3& n, const Vec3& s, const Vec3& t) {
    return Vec3(s.x*w.x + n.x*w.y + t.x*w.z, s.y*w.x + n.y*w.y + t.y*w.z, s.z*w.x + n.z*w.y + t.z*w.z);
}
#endif
