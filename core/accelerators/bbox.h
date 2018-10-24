#pragma once
#include "Ray.h"
#include "../../extern/glm/glm/glm.hpp"
#include <stdint.h>

struct BBox {
  glm::vec3 min, max, extent;
  BBox() { }
  BBox(const glm::vec3& min, const glm::vec3& max);
  BBox(const glm::vec3& p);

  bool intersect(const Ray& ray, float *tnear, float *tfar) const;
  void expandToInclude(const glm::vec3& p);
  void expandToInclude(const BBox& b);
  uint32_t maxDimension() const;
  float surfaceArea() const;
};
