#include "bvh.h"
#include <stdio.h>
//#ifndef PBRT_L1_CACHE_LINE_SIZE
//#define PBRT_L1_CACHE_LINE_SIZE 64
//#endif

// Feel free to ignore these structs entirely!
// They are here if you want to implement any of PBRT's
// methods for BVH construction.
//STAT_MEMORY_COUNTER("Memory/BVH tree", treeBytes);
//#define CHECK_EQ(x, y) CHECK((x) == (y))
//#define CHECK_NE(x, y) CHECK((x) != (y))
//#define CHECK_LE(x, y) CHECK((x) <= (y))
//#define CHECK_LT(x, y) CHECK((x) < (y))
//#define CHECK_GE(x, y) CHECK((x) >= (y))
//#define CHECK_GT(x, y) CHECK((x) > (y))
struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f &bounds)
        : primitiveNumber(primitiveNumber),
          bounds(bounds),
          centroid(.5f * bounds.min + .5f * bounds.max) {}
    int primitiveNumber;
    Bounds3f bounds;
    Point3f centroid;
};

struct BVHBuildNode {
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const Bounds3f &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
    Bounds3f bounds;
    BVHBuildNode *children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
};

struct MortonPrimitive {
    int primitiveIndex;
    unsigned int mortonCode;
};

struct LBVHTreelet {
    int startIndex, nPrimitives;
    BVHBuildNode *buildNodes;
};

struct LinearBVHNode {
    Bounds3f bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    unsigned short nPrimitives;  // 0 -> interior node, 16 bytes
    unsigned char axis;          // interior node: xyz, 8 bytes
    unsigned char pad[1];        // ensure 32 byte total size
};


BVHAccel::~BVHAccel()
{
    delete [] nodes;
}

// Constructs an array of BVHPrimitiveInfos, recursively builds a node-based BVH
// from the information, then optimizes the memory of the BVH
BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive> > &p, int maxPrimsInNode)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(p)
{

    //TODO
        if (primitives.empty()) return;
        // Build BVH from _primitives_

        // Initialize _primitiveInfo_ array for primitives
        std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
        for (size_t i = 0; i < primitives.size(); ++i)
            primitiveInfo[i] = {i, primitives[i]->shape->WorldBound()};

        // Build BVH tree for primitives using _primitiveInfo_
        //MemoryArena arena(1024 * 1024);
        int totalNodes = 0;
        std::vector<std::shared_ptr<Primitive>> orderedPrims;
        orderedPrims.reserve(primitives.size());
        BVHBuildNode *root;
        root = recursiveBuild(primitiveInfo, 0, primitives.size(),
                                  &totalNodes, orderedPrims);
        primitives.swap(orderedPrims);
//        LOG(INFO) << StringPrintf("BVH created with %d nodes for %d "
//                                  "primitives (%.2f MB)", totalNodes,
//                                  (int)primitives.size(),
//                                  float(totalNodes * sizeof(LinearBVHNode)) /
//                                  (1024.f * 1024.f));

        // Compute representation of depth-first traversal of BVH tree
//        treeBytes += totalNodes * sizeof(LinearBVHNode) + sizeof(*this) +
//                     primitives.size() * sizeof(primitives[0]);
        //nodes = AllocAligned<LinearBVHNode>(totalNodes);
        //nodes=(LinearBVHNode*)_aligned_malloc(totalNodes*sizeof(LinearBVHNode),PBRT_L1_CACHE_LINE_SIZE);
        nodes= new LinearBVHNode[totalNodes];
        int offset = 0;
        flattenBVHTree(root, &offset);
        //CHECK_EQ(totalNodes, offset);
        if(totalNodes!=offset)
            return;
}

bool BVHAccel::Intersect(const Ray &ray, Intersection *isect) const
{
    //TODO
       //return false;
    if (!nodes) return false;
        //ProfilePhase p(Prof::AccelIntersect);
        bool hit = false;
        Vector3f invDir(1 / ray.direction.x, 1 / ray.direction.y, 1 / ray.direction.z);
        int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
        // Follow ray through BVH nodes to find primitive intersections
        int toVisitOffset = 0, currentNodeIndex = 0;
        int nodesToVisit[64];
        float minT=INFINITE;
        Intersection minIsect;
        while (true) {
            const LinearBVHNode *node = &nodes[currentNodeIndex];
            // Check ray against BVH node
            if (node->bounds.IntersectP(ray, invDir, dirIsNeg)) {
                if (node->nPrimitives > 0) {
                    // Intersect ray with primitives in leaf BVH node
                    for (int i = 0; i < node->nPrimitives; ++i)
                        if (primitives[node->primitivesOffset + i]->Intersect(
                                ray, isect))
                        {

                            if(minT>isect->t)
                            {
                                minT=isect->t;
                                minIsect=*isect;
                            }
                            hit=true;

                        }
                    if (toVisitOffset == 0) break;
                    currentNodeIndex = nodesToVisit[--toVisitOffset];
                } else {
                    // Put far BVH node on _nodesToVisit_ stack, advance to near
                    // node
                    if (dirIsNeg[node->axis]) {
                        nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                        currentNodeIndex = node->secondChildOffset;
                    } else {
                        nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                        currentNodeIndex = currentNodeIndex + 1;
                    }
                }
            } else {
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
        }
        *isect=minIsect;
        return hit;
}
BVHBuildNode* BVHAccel::recursiveBuild(
    std::vector<BVHPrimitiveInfo> &primitiveInfo,
    int start, int end, int *totalNodes,
    std::vector<std::shared_ptr<Primitive>> &orderedPrims)
{


    //CHECK_NE(start, end);
    if(start==end) return nullptr;
    //BVHBuildNode *node = arena.Alloc<BVHBuildNode>();
    BVHBuildNode *node = new BVHBuildNode();
    (*totalNodes)++;
    // Compute bounds of all primitives in BVH node
    Bounds3f bounds;
    for (int i = start; i < end; ++i)
        bounds = Union(bounds, primitiveInfo[i].bounds);
    int nPrimitives = end - start;
    if (nPrimitives == 1) {
        // Create leaf _BVHBuildNode_
        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    } else {
        // Compute bound of primitive centroids, choose split dimension _dim_
        //Bounds3f centroidBounds;
        Bounds3f centroidBounds=Bounds3f(primitiveInfo[start].centroid);
        for (int i = start; i < end; ++i)
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        int dim = centroidBounds.MaximumExtent();

        // Partition primitives into two sets and build children
        int mid = (start + end) / 2;
        if (centroidBounds.max[dim] == centroidBounds.min[dim]) {
            // Create leaf _BVHBuildNode_
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        } else {
            // Partition primitives based on _splitMethod_

                // Partition primitives using approximate SAH
                if (nPrimitives <= 2) {
                    // Partition primitives into equally-sized subsets
                    mid = (start + end) / 2;
                    std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                                     &primitiveInfo[end - 1] + 1,
                                     [dim](const BVHPrimitiveInfo &a,
                                           const BVHPrimitiveInfo &b) {
                                         return a.centroid[dim] <
                                                b.centroid[dim];
                                     });
                } else {
                    // Allocate _BucketInfo_ for SAH partition buckets
                    //PBRT_CONSTEXPR int nBuckets = 12;
                    //BucketInfo buckets[nBuckets];
                    constexpr int nBuckets = 12;
                    struct BucketInfo {
                    int count = 0;
                    Bounds3f bounds;
                    };
                    BucketInfo buckets[nBuckets];

                    // Initialize _BucketInfo_ for SAH partition buckets
                    for (int i = start; i < end; ++i) {
                        int b = nBuckets *
                                centroidBounds.Offset(
                                    primitiveInfo[i].centroid)[dim];
                        if (b == nBuckets) b = nBuckets - 1;
                        //CHECK_GE(b, 0);
                        //CHECK_LT(b, nBuckets);
                        if(b>=0){
                            if(b<nBuckets){
                        buckets[b].count++;
                        buckets[b].bounds =
                            Union(buckets[b].bounds, primitiveInfo[i].bounds);}}
                    }

                    // Compute costs for splitting after each bucket
                    Float cost[nBuckets - 1];
                    for (int i = 0; i < nBuckets - 1; ++i) {
                        Bounds3f b0, b1;
                        int count0 = 0, count1 = 0;
                        for (int j = 0; j <= i; ++j) {
                            b0 = Union(b0, buckets[j].bounds);
                            count0 += buckets[j].count;
                        }
                        for (int j = i + 1; j < nBuckets; ++j) {
                            b1 = Union(b1, buckets[j].bounds);
                            count1 += buckets[j].count;
                        }
                        cost[i] = 1 +
                                  (count0 * b0.SurfaceArea() +
                                   count1 * b1.SurfaceArea()) /
                                      bounds.SurfaceArea();
                    }

                    // Find bucket to split at that minimizes SAH metric
                    Float minCost = cost[0];
                    int minCostSplitBucket = 0;
                    for (int i = 1; i < nBuckets - 1; ++i) {
                        if (cost[i] < minCost) {
                            minCost = cost[i];
                            minCostSplitBucket = i;
                        }
                    }

                    // Either create leaf or split primitives at selected SAH
                    // bucket
                    Float leafCost = nPrimitives;
                    if (nPrimitives > maxPrimsInNode || minCost < leafCost) {
                        BVHPrimitiveInfo *pmid = std::partition(
                            &primitiveInfo[start], &primitiveInfo[end - 1] + 1,
                            [=](const BVHPrimitiveInfo &pi) {
                                int b = nBuckets *
                                        centroidBounds.Offset(pi.centroid)[dim];
                                if (b == nBuckets) b = nBuckets - 1;
                                //CHECK_GE(b, 0);
                                //CHECK_LT(b, nBuckets);
                                if(b>=0){
                                    if(b<nBuckets)
                                return b <= minCostSplitBucket;}
                            });
                        mid = pmid - &primitiveInfo[0];
                    } else {
                        // Create leaf _BVHBuildNode_
                        int firstPrimOffset = orderedPrims.size();
                        for (int i = start; i < end; ++i) {
                            int primNum = primitiveInfo[i].primitiveNumber;
                            orderedPrims.push_back(primitives[primNum]);
                        }
                        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
                        return node;
                    }
                }
                //break;
            if(mid==start) mid++;
            if(mid==end) mid--;

            node->InitInterior(dim,
                               recursiveBuild(primitiveInfo, start, mid,
                                              totalNodes, orderedPrims),
                               recursiveBuild(primitiveInfo, mid, end,
                                              totalNodes, orderedPrims));
        }
    }
    return node;
}

int BVHAccel::flattenBVHTree(BVHBuildNode *node, int *offset)
{
    LinearBVHNode *linearNode = &nodes[*offset];
        linearNode->bounds = node->bounds;
        int myOffset = (*offset)++;
        if (node->nPrimitives > 0) {
            //CHECK(!node->children[0] && !node->children[1]);
            //CHECK_LT(node->nPrimitives, 65536);
            if(!node->children[0] && !node->children[1]){
                if(node->nPrimitives< 65536){
            linearNode->primitivesOffset = node->firstPrimOffset;
            linearNode->nPrimitives = node->nPrimitives;}}
        } else {
            // Create interior flattened BVH node
            linearNode->axis = node->splitAxis;
            linearNode->nPrimitives = 0;
            flattenBVHTree(node->children[0], offset);
            linearNode->secondChildOffset =
                flattenBVHTree(node->children[1], offset);
        }
        return myOffset;
}
