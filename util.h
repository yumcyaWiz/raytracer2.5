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
#endif
