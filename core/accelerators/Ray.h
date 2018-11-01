#pragma once
#include "common.h"

struct Ray {
  glm::vec3 o; // Ray Origin
  glm::vec3 d; // Ray Direction
  glm::vec3 inv_d; // Inverse of each Ray Direction component

  Ray(const glm::vec3& o, const glm::vec3& d)
    : o(o), d(d), inv_d(glm::vec3(1,1,1) / d) { }
};