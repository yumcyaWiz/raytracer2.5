#ifndef ACCEL_H
#define ACCEL_H
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "aabb.h"
template <typename T>
class Accel {
    public:
        std::vector<std::shared_ptr<T>> prims;

        Accel() {};
        Accel(const std::vector<std::shared_ptr<T>> &_prims) : prims(_prims) {};
        virtual ~Accel() {};

        virtual bool intersect(const Ray& ray, Hit& res) const = 0;
        virtual AABB worldBound() const = 0;
};


enum class BVH_PARTITION_TYPE {
    EQSIZE,
    CENTER,
    SAH
};


template <typename T>
class BVH : public Accel<T> {
    public:
        struct BVHNode {
            AABB bbox;
            BVHNode* left;
            BVHNode* right;
            int splitAxis;
            int indexOffset;
            int nPrims;

            BVHNode() {};

            void initLeaf(int _indexOffset, int _nPrims, const AABB& _bbox) {
                bbox = _bbox;
                left = right = nullptr;
                indexOffset = _indexOffset;
                nPrims = _nPrims;
            };
            void initNode(int _splitAxis, BVHNode* _left, BVHNode* _right) {
                bbox = mergeAABB(_left->bbox, _right->bbox);
                left = _left;
                right = _right;
                splitAxis = _splitAxis;
                nPrims = 0;
            };
        };

        
        struct BVHPrimitiveInfo {
            int primIndex;
            AABB bbox;
            Vec3 centroid;

            BVHPrimitiveInfo() {};
            BVHPrimitiveInfo(int _primIndex, const AABB& _bbox) : primIndex(_primIndex), bbox(_bbox), 
            centroid(0.5f*_bbox.pMin + 0.5f*_bbox.pMax) {};
        };


        BVHNode* bvh_root;
        const int maxPrimsInLeaf;
        const BVH_PARTITION_TYPE ptype;
        int totalNodes;

        
        struct linearBVHNode {
            AABB bbox;
            union {
                int indexOffset;
                int rightChildOffset;
            };
            uint16_t nPrims;
            uint8_t splitAxis;
            uint8_t pad[1];
        };
        linearBVHNode *linearNodes;


        BVH(const std::vector<std::shared_ptr<T>>& _prims, int _maxPrimsInLeaf, BVH_PARTITION_TYPE _ptype) : Accel<T>(_prims), maxPrimsInLeaf(_maxPrimsInLeaf), ptype(_ptype) {
            totalNodes = 0;
            constructBVH();
        };
        ~BVH() {
            delete[] linearNodes;
        };


        void constructBVH() {
            if(this->prims.size() == 0) {
                std::cerr << "prims is empty!" << std::endl;
                std::exit(1);
            }

            std::vector<BVHPrimitiveInfo> primitiveInfo(this->prims.size());
            for(size_t i = 0; i < this->prims.size(); i++) {
                primitiveInfo[i] = BVHPrimitiveInfo(i, this->prims[i]->worldBound());
            }

            std::vector<std::shared_ptr<T>> orderedPrims;

            bvh_root = makeBVHNode(0, this->prims.size(), primitiveInfo, orderedPrims, ptype, &totalNodes);
            this->prims.swap(orderedPrims);
            std::cout << "BVH Construction Finished!" << std::endl;
            std::cout << "BVH Nodes:" << totalNodes << std::endl;

            linearNodes = new linearBVHNode[totalNodes];
            int offset = 0;
            makeLinearBVHNode(bvh_root, &offset);
            std::cout << "LinearBVH Construction Finished!" << std::endl;
        };


        BVHNode* makeBVHNode(int start, int end, std::vector<BVHPrimitiveInfo> &primitiveInfo, std::vector<std::shared_ptr<T>>& orderedPrims, BVH_PARTITION_TYPE ptype, int *totalNodes) {
            (*totalNodes)++;
            BVHNode* node = new BVHNode();

            int nPrims = end - start;

            AABB bounds;
            for(int i = start; i < end; i++) {
                bounds = mergeAABB(bounds, primitiveInfo[i].bbox);
            }

            if(nPrims <= maxPrimsInLeaf) {
                int indexOffset = orderedPrims.size();
                for(int i = start; i < end; i++)
                    orderedPrims.push_back(this->prims[primitiveInfo[i].primIndex]);
                node->initLeaf(indexOffset, nPrims, bounds);
                return node;
            }

            AABB centroidBounds;
            for(int i = start; i < end; i++)
                centroidBounds = mergeAABB(centroidBounds, primitiveInfo[i].centroid);

            int axis = maximumExtent(centroidBounds);

            if(centroidBounds.pMin[axis] == centroidBounds.pMax[axis]) {
                int indexOffset = orderedPrims.size();
                for(int i = start; i < end; i++)
                    orderedPrims.push_back(this->prims[primitiveInfo[i].primIndex]);
                node->initLeaf(indexOffset, nPrims, bounds);
                return node;
            }

            int mid = (start + end)/2;
            switch(ptype) {
                case BVH_PARTITION_TYPE::CENTER:
                    {
                        float midPoint = 0.5f*centroidBounds.pMin[axis] + 0.5f*centroidBounds.pMax[axis];
                        BVHPrimitiveInfo* midPtr = std::partition(&primitiveInfo[start], &primitiveInfo[end-1]+1, [axis, midPoint](const BVHPrimitiveInfo& x) {
                                return x.centroid[axis] < midPoint;
                                });
                        mid = midPtr - &primitiveInfo[0];

                        if(mid != start && mid != end) break;
                    }
                case BVH_PARTITION_TYPE::EQSIZE:
                    {
                        mid = (start + end)/2;
                        std::nth_element(&primitiveInfo[start], &primitiveInfo[end], &primitiveInfo[end-1]+1, [axis](const BVHPrimitiveInfo& x, const BVHPrimitiveInfo& y) {
                                return x.centroid[axis] < y.centroid[axis];
                                });
                        break;
                    }
                default:
                    {
                        constexpr int nBuckets = 12;
                        struct BucketInfo {
                            int primCount = 0;
                            AABB bbox;
                        };

                        BucketInfo buckets[nBuckets];
                        for(int i = start; i < end; i++) {
                            AABB prim_bbox = primitiveInfo[i].bbox;
                            int b = nBuckets * centroidBounds.offset(primitiveInfo[i].centroid)[axis];
                            if(b == nBuckets) b = nBuckets - 1;
                            buckets[b].primCount++;
                            buckets[b].bbox = mergeAABB(buckets[b].bbox, prim_bbox);
                        }

                        float cost[nBuckets - 1];
                        for(int i = 0; i < nBuckets - 1; i++) {
                            int count0 = 0;
                            int count1 = 0;
                            AABB b0, b1;
                            for(int j = 0; j <= i; j++) {
                                count0 += buckets[j].primCount;
                                b0 = mergeAABB(b0, buckets[j].bbox);
                            }
                            for(int j = i+1; j < nBuckets - 1; j++) {
                                count1 += buckets[j].primCount;
                                b1 = mergeAABB(b1, buckets[j].bbox);
                            }

                            float b0area = b0.surfaceArea();
                            float b1area = b1.surfaceArea();
                            if(std::isinf(b0area))
                                b0area = 100000.0f;
                            if(std::isinf(b1area))
                                b1area = 100000.0f;
                            cost[i] = 0.125f + (count0*b0area + count1*b1area)/bounds.surfaceArea();
                        }

                        float minCost = cost[0];
                        int splitPosition = 0;
                        for(int i = 1; i < nBuckets - 1; i++) {
                            if(cost[i] < minCost) {
                                minCost = cost[i];
                                splitPosition = i;
                            }
                        }

                        float leafCost = nPrims;
                        if(minCost < leafCost) {
                            BVHPrimitiveInfo* midPtr = std::partition(&primitiveInfo[start], &primitiveInfo[end-1]+1, [=](const BVHPrimitiveInfo& x) {
                                    int b = nBuckets * centroidBounds.offset(x.centroid)[axis];
                                    if(b == nBuckets) b = nBuckets - 1;
                                    return b <= splitPosition;
                                    });
                            mid = midPtr - &primitiveInfo[0];
                        }
                        else {
                            int indexOffset = orderedPrims.size();
                            for(int i = start; i < end; i++)
                                orderedPrims.push_back(this->prims[primitiveInfo[i].primIndex]);
                            node->initLeaf(indexOffset, nPrims, bounds);
                            return node;
                        }
                    }
            }

            BVHNode* node_left = makeBVHNode(start, mid, primitiveInfo, orderedPrims, ptype, totalNodes);
            BVHNode* node_right = makeBVHNode(mid, end, primitiveInfo, orderedPrims, ptype, totalNodes);
            node->initNode(axis, node_left, node_right);
            return node;
        };


        int makeLinearBVHNode(BVHNode* node, int *offset) {
            linearBVHNode* linearNode = &linearNodes[*offset];

            linearNode->bbox = node->bbox;
            int _offset = (*offset)++;
            if(node->nPrims > 0) {
                linearNode->indexOffset = node->indexOffset;
                linearNode->nPrims = node->nPrims;
            }
            else {
                linearNode->splitAxis = node->splitAxis;
                linearNode->nPrims = 0;
                makeLinearBVHNode(node->left, offset);
                linearNode->rightChildOffset = makeLinearBVHNode(node->right, offset);
            }
            return _offset;
        };


        bool intersect(const Ray& ray, Hit& isect) const {
            Vec3 invDir = 1.0f/(ray.direction);
            if(std::abs(invDir.x) > ray.tmax) invDir.x += sign(invDir.x)*ray.tmax;
            if(std::abs(invDir.y) > ray.tmax) invDir.y += sign(invDir.y)*ray.tmax;
            if(std::abs(invDir.z) > ray.tmax) invDir.z += sign(invDir.z)*ray.tmax;
            const int dirIsNeg[3] = {ray.direction.x < 0, ray.direction.y < 0, ray.direction.z < 0};
            return intersect(ray, isect, invDir, dirIsNeg);
        };
        bool intersect(const Ray& ray, Hit& isect, const Vec3& invDir, const int dirIsNeg[3]) const {
            bool hit = false;
            int toVisitOffset = 0, currentNodeIndex = 0;
            int nodesToVisit[64];
            while(true) {
                const linearBVHNode* node = &linearNodes[currentNodeIndex];
                if(node->bbox.intersect(ray, invDir, dirIsNeg)) {
                    if(node->nPrims > 0) {
                        for(size_t i = 0; i < node->nPrims; i++) {
                            const int index = node->indexOffset + i;
                            Hit isect2;
                            if(this->prims[index]->intersect(ray, isect2)) {
                                hit = true;
                                //Primitiveに対して衝突計算するときは最大距離がPrimitive::intersectの方で自動更新されるので不要だが、Shapeに対して衝突計算するときにはShape::intersect内で最大距離が更新されないので必要
                                ray.tmax = isect2.t < ray.tmax ? isect2.t : ray.tmax;
                                isect = isect2;
                            }
                        }
                        if(toVisitOffset == 0) break;
                        currentNodeIndex = nodesToVisit[--toVisitOffset];
                    }
                    else {
                        if(dirIsNeg[node->splitAxis]) {
                            nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                            currentNodeIndex = node->rightChildOffset;
                        }
                        else {
                            nodesToVisit[toVisitOffset++] = node->rightChildOffset;
                            currentNodeIndex++;
                        }
                    }
                }
                else {
                    if(toVisitOffset == 0) break;
                    currentNodeIndex = nodesToVisit[--toVisitOffset];
                }
            }
            return hit;
        };


        AABB worldBound() const {
            AABB bounds;
            for(auto itr = this->prims.begin(); itr != this->prims.end(); itr++) {
                bounds = mergeAABB(bounds, (*itr)->worldBound());
            }
            return bounds;
        };
};
#endif
