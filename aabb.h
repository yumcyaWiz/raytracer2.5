#ifndef AABB_H
#define AABB_H
#include <limits>
#include "vec3.h"
class AABB {
    public:
        Vec3 pMin;
        Vec3 pMax;


        AABB() {
            constexpr float INF_MIN = std::numeric_limits<float>::lowest();
            constexpr float INF_MAX = std::numeric_limits<float>::max();
            pMin = Vec3(INF_MAX);
            pMax = Vec3(INF_MIN);
        };
        AABB(const Vec3& p1, const Vec3& p2) {
            pMin = min(p1, p2);
            pMax = max(p1, p2);
        };


        Vec3 operator[](int i) const {
            return i == 0 ? pMin : pMax;
        };

        
        float surfaceArea() const {
            const float dx = pMax.x - pMin.x;
            const float dy = pMax.y - pMin.y;
            const float dz = pMax.z - pMin.z;
            return 2*(dx*dy + dy*dz + dz*dx);
        };


        bool intersect(const Ray& ray) const {
            float t0 = ray.tmin;
            float t1 = ray.tmax;
            for(int i = 0; i < 3; i++) {
                const float invDir = 1.0f/ray.direction[i];
                float tNear = (pMin[i] - ray.origin[i])*invDir;
                float tFar = (pMax[i] - ray.origin[i])*invDir;

                if(tNear > tFar) std::swap(tNear, tFar);

                t0 = tNear > t0 ? tNear : t0;
                t1 = tFar < t1 ? tFar : t1;
                if(t0 > t1) return false;
            }
            return true;
        };
        bool intersect(const Ray& ray, const Vec3& invDir, const int dirIsNeg[3]) const {
            const AABB& bounds = *this;

            float tMin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
            float tMax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
            const float tyMin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
            const float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;
            if(tMin > tyMax || tyMin > tMax) return false;
            if(tyMin > tMin) tMin = tyMin;
            if(tyMax < tMax) tMax = tyMax;

            const float tzMin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
            const float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;
            if(tMin > tzMax || tzMin > tMax) return false;
            if(tzMin > tMin) tMin = tzMin;
            if(tzMax < tMax) tMax = tzMax;

            return (tMin < ray.tmax) && (tMax >= ray.tmin);
        };


        Vec3 offset(const Vec3& p) const {
            Vec3 o = p - pMin;
            o.x /= (pMax.x - pMin.x);
            o.y /= (pMax.y - pMin.y);
            o.z /= (pMax.z - pMin.z);
            return o;
        };
};


inline std::ostream& operator<<(std::ostream& stream, const AABB& aabb) {
    stream << "pMin:" << aabb.pMin << std::endl;
    stream << "pMax:" << aabb.pMax << std::endl;
    return stream;
}


inline AABB mergeAABB(const AABB& b1, const AABB& b2) {
    return AABB(min(b1.pMin, b2.pMin), max(b1.pMax, b2.pMax));
}
inline AABB mergeAABB(const AABB& b, const Vec3& p) {
    return AABB(min(b.pMin, p), max(b.pMax, p));
}


inline int maximumExtent(const AABB& b) {
    float dx = b.pMax.x - b.pMin.x;
    float dy = b.pMax.y - b.pMin.y;
    float dz = b.pMax.z - b.pMin.z;
    float max = std::max(dx, std::max(dy, dz));
    if(max == dx)
        return 0;
    else if(max == dy)
        return 1;
    else
        return 2;
}
#endif
