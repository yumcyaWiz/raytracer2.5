#ifndef UTIL_H
#define UTIL_H
template <typename T>
T clamp(T x, T min, T max) {
    if(x < min)
        return min;
    else if(x > max)
        return max;
    else 
        return x;
}
#endif
