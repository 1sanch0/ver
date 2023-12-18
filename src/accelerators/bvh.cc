#include "bvh.hh"


BVH::BVH(std::vector<std::shared_ptr<Primitive>> &&p, size_t maxPrimsNode)
  : primitives{std::move(p)}, nodes{}, maxPrimsInNode(std::min((size_t)255, maxPrimsNode)) {
  
  if (primitives.empty()) return;

  std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
  for (size_t i = 0; i < primitives.size(); i++)
    primitiveInfo[i] = BVHPrimitiveInfo(i, primitives[i]->bounds());

  uint totalNodes = 0;
  std::vector<std::shared_ptr<Primitive>> orderedPrims;
  orderedPrims.reserve(primitives.size());

  std::shared_ptr<BVHNode> root = build(0, primitives.size(), totalNodes, primitiveInfo, orderedPrims);

  primitives.swap(orderedPrims);
  primitiveInfo.resize(0); // TODO: useless
  
  nodes.resize(totalNodes);
  uint offset = 0;
  flattenBVHTree(root, offset);
  assert(totalNodes == offset, "notalNodes==offset");
  // TODO: CHECK
}

Bounds BVH::bounds() const { return nodes.empty() ? Bounds() : nodes[0].bounds; }

std::shared_ptr<BVHNode> BVH::build(uint start, uint end, uint &totalNodes,
                                    std::vector<BVHPrimitiveInfo> &primInfo,
                                    std::vector<std::shared_ptr<Primitive>> &orderedPrims) {
  assert(start != end, "start != end");
  totalNodes++;

  Bounds bounds;
  for (uint i = start; i < end; i++)
    bounds = bounds.Union(primInfo[i].bounds);

  uint nPrims = end - start;
  if (nPrims == 1) { // Leaf
    uint firstPrimOffset = orderedPrims.size();
    for (uint i = start; i < end; i++) {
      uint primIdx = primInfo[i].idx;
      orderedPrims.push_back(primitives[primIdx]);
    }
    return std::make_shared<BVHNode>(firstPrimOffset, nPrims, bounds);
  }

  Bounds centroidBounds;
  for (uint i = start; i < end; i++)
    centroidBounds = centroidBounds.Union(primInfo[i].centroid);
  uint dim = centroidBounds.maximumExtent(); 

  uint mid = (start + end) / 2;
  if (centroidBounds.max[dim] == centroidBounds.min[dim]) { // Leaf
    uint firstPrimOffset = orderedPrims.size();
    for (uint i = start; i < end; i++) {
      uint primIdx = primInfo[i].idx;
      orderedPrims.push_back(primitives[primIdx]);
    }
    return std::make_shared<BVHNode>(firstPrimOffset, nPrims, bounds);
  } 
  
  #if 0 // Middle
    Float pmid = (centroidBounds.min[dim] + centroidBounds.max[dim]) / 2;
    BVHPrimitiveInfo *midPtr = std::partition(
        &primInfo[start], &primInfo[end - 1] + 1,
        [dim, pmid](const BVHPrimitiveInfo &pi) {
            return pi.centroid[dim] < pmid;
        });
    mid = midPtr - &primInfo[0];
    // For lots of prims with large overlapping bounding boxes, this
    // may fail to partition; in that case don't break and fall
    // through
    // to EqualCounts.
    if (mid != start && mid != end) {

    } else {
      mid = (start + end) / 2;
      std::nth_element(&primInfo[start], &primInfo[mid], &primInfo[end-1]+1,
        [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
          return a.centroid[dim] < b.centroid[dim];
        });
    }
  #endif
  #if 1 // EQUAL COUNTS
    mid = (start + end) / 2;
    std::nth_element(&primInfo[start], &primInfo[mid], &primInfo[end-1]+1,
      [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
        return a.centroid[dim] < b.centroid[dim];
      });
  #endif

  // TODO: SAH 

  return std::make_shared<BVHNode>(dim,
                                   build(start, mid, totalNodes, primInfo, orderedPrims),
                                   build(mid, end, totalNodes, primInfo, orderedPrims));
}

uint BVH::flattenBVHTree(const std::shared_ptr<BVHNode> &node, uint &offset) {
  LinearBVHNode &linearNode = nodes[offset];
  linearNode.bounds = node->bounds;

  uint off = offset++;

  if (node->nPrimitives > 0) {
    assert(!node->children[0] && !node->children[1], "nullptr children");
    assert(node->nPrimitives < 65536, "TODO");
    linearNode.offset = node->fPrimOffset;
    linearNode.nPrims = node->nPrimitives;
  } else {
    linearNode.axis = node->splitAxis;
    linearNode.nPrims = 0;
    flattenBVHTree(node->children[0], offset);
    linearNode.offset = flattenBVHTree(node->children[1], offset);
  }

  return off;
}

bool BVH::intersect(const Ray &ray, SurfaceInteraction &interact) const {
  if (nodes.empty()) return false;
  bool hit = false;
  Direction invDir(1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z);
  int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

  uint toVisitOffset = 0, currentNodeIndex = 0;
  uint nodesToVisit[64];

  SurfaceInteraction tmpInteract;
  tmpInteract.t = std::numeric_limits<Float>::max();
  interact.t = std::numeric_limits<Float>::max();
  for(;;) {
    const LinearBVHNode &node = nodes[currentNodeIndex];
    if (node.bounds.intersect(ray, invDir, dirIsNeg)) {
      if (node.nPrims > 0) {
        for (uint i = 0; i < node.nPrims; i++)
          if (primitives[node.offset + i]->intersect(ray, tmpInteract))
            if (tmpInteract.t < interact.t) {
              hit = true;
              interact = tmpInteract;
            }
        if (toVisitOffset == 0)
          break;
        currentNodeIndex = nodesToVisit[--toVisitOffset];
      } else {
        if (dirIsNeg[node.axis]) {
          nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
          currentNodeIndex = node.offset;
        } else {
          nodesToVisit[toVisitOffset++] = node.offset;
          currentNodeIndex = currentNodeIndex + 1;
        }
      }
    } else {
      if (toVisitOffset == 0)
        break;
      currentNodeIndex = nodesToVisit[--toVisitOffset];
    }
  }
  return hit;
}
