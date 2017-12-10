#ifndef Object_h_
#define Object_h_

#include "IntersectionInfo.h"
//#include "Ray.h"
#include "BBox.h"
#include "../../extern/glm/glm/vec3.hpp"

struct Object {
  //! All "Objects" must be able to test for intersections with rays.
  virtual bool getIntersection(
      const glm::vec3 ori, const glm::vec3 dir,
      IntersectionInfo* intersection)
    const = 0;

  //! Return an object normal based on an intersection
  virtual glm::vec3 getNormal(const IntersectionInfo& I) const = 0;

  //! Return a bounding box for this object
  virtual BBox getBBox() const = 0;

  //! Return the centroid for this object. (Used in BVH Sorting)
  virtual glm::vec3 getCentroid() const = 0;
};

#endif
