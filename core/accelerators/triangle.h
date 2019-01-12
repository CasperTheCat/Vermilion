#pragma once

#include "object.h"

class Triangle : public Object
{
    glm::vec3 v0,v1,v2;
    glm::vec3 v0n,v1n,v2n;
    glm::vec2 v0uv, v1uv, v2uv;

public:
    Triangle(
        const glm::vec3& _v0,
        const glm::vec3& _v1,
        const glm::vec3& _v2,

        const glm::vec3& _v0n,
        const glm::vec3& _v1n,
        const glm::vec3& _v2n,

        const glm::vec2& _v0uv,
        const glm::vec2& _v1uv,
        const glm::vec2& _v2uv
        ) : 
        v0(_v0),
        v1(_v1),
        v2(_v2),

        v0n(_v0n),
        v1n(_v1n),
        v2n(_v2n),

        v0uv(_v0uv),
        v1uv(_v1uv),
        v2uv(_v2uv)
    {}

    bool getIntersection(const Ray& ray, IntersectionInfo* intersection) const override;
    virtual glm::vec3 getNormal(const IntersectionInfo& I, glm::vec2 *pUV) const override;
    virtual BBox getBBox() const override;
    virtual glm::vec3 getCentroid() const override;
};