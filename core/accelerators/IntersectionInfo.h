#pragma once
#include "common.h"

class Object;

struct IntersectionInfo {
  float t; // Intersection distance along the ray
  const Object* object; // Object that was hit
  glm::vec3 hit; // Location of the intersection
};