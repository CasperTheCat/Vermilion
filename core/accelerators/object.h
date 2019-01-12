#pragma once

#include "IntersectionInfo.h"
#include "Ray.h"
#include "bbox.h"

class Object {
    public:
  //! All "Objects" must be able to test for intersections with rays.
  virtual bool getIntersection(const Ray& ray, IntersectionInfo* intersection) const = 0;

  //! Return an object normal based on an intersection
  virtual glm::vec3 getNormal(const IntersectionInfo& I, glm::vec2 *pUV) const = 0;

  //! Return a bounding box for this object
  virtual BBox getBBox() const = 0;

  //! Return the centroid for this object. (Used in BVH Sorting)
  virtual glm::vec3 getCentroid() const = 0;
};

