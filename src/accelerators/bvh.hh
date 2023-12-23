#ifndef BVH_H_
#define BVH_H_

#include "ver.hh"
#include "shapes/primitive.hh"

struct BVHPrimitiveInfo {
  BVHPrimitiveInfo() = default;
  BVHPrimitiveInfo(size_t primIdx, const Bounds &b)
    : idx{primIdx}, bounds{b}, centroid{Direction(b.min) * 0.5 + Direction(b.max) * 0.5} {}

  size_t idx;
  Bounds bounds;
  Point centroid;
};

struct BVHNode {
  BVHNode(uint first, uint n, const Bounds &b)
    : bounds{b}, children{nullptr, nullptr},
      fPrimOffset{first}, nPrimitives{n} {}
  
  BVHNode(uint axis, 
          const std::shared_ptr<BVHNode> c0, 
          const std::shared_ptr<BVHNode> c1)
    : bounds{c0->bounds.Union(c1->bounds)},
      children{c0, c1}, splitAxis{axis},
      fPrimOffset{0}, nPrimitives{0} {}

  Bounds bounds;
  std::shared_ptr<BVHNode> children[2];
  uint splitAxis, fPrimOffset, nPrimitives;
};

struct LinearBVHNode {
  Bounds bounds;
  uint offset;
  uint16_t nPrims;
  uint16_t axis;
};

class BVH : public Primitive {
  public:
    BVH(std::vector<std::shared_ptr<Primitive>> &&p, size_t maxPrimsNode = 255);
    Bounds bounds() const;
    bool intersect(const Ray &ray, SurfaceInteraction &interact) const;

  private:
    std::shared_ptr<BVHNode> build(uint start, uint end, uint &totalNodes,
                                   std::vector<BVHPrimitiveInfo> &primInfo,
                                   std::vector<std::shared_ptr<Primitive>> &orderedPrims); 

    uint flattenBVHTree(const std::shared_ptr<BVHNode> &node, uint &offset);
  private:
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<LinearBVHNode> nodes;
    size_t maxPrimsInNode;
};

#endif // BVH_H_
