#include "triangle.h"

bool Triangle::getIntersection(const Ray& ray, IntersectionInfo* intersection) const
{
	using namespace glm;

    vec3 rot = ray.d;
    vec3 pos = ray.o;

	// Triangle Intersection REMOVE FOR PUSH
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	// Calculate planes normal vector
	vec3 pvec = cross(rot, e2);
	float det = dot(e1, pvec);

	// Ray is parallel to plane
	if (det < 1e-8 && det > -1e-8) {
		return false;
	}

	float inv_det = 1 / det;
	vec3 tvec = pos - v0;
	float u = dot(tvec, pvec) * inv_det;
	if (u < 0 || u > 1) {
		return false;
	}

	vec3 qvec = cross(tvec, e1);
	float v = dot(rot, qvec) * inv_det;
	if (v < 0 || u + v > 1) {
		return false;
	}

	auto dist = dot(e2, qvec) * inv_det;

	if(dist > 0.0f)
	{
		intersection->t = dist;
		intersection->object = this;
		return true;
	}

	return false;


}

glm::vec3 Triangle::getNormal(const IntersectionInfo& I, glm::vec2 *pUV) const
{
    glm::vec3 f0 = v0 - I.hit;
	glm::vec3 f1 = v1 - I.hit;
	glm::vec3 f2 = v2 - I.hit;

    auto w = glm::length(glm::cross(v0 - v1, v0 - v2));
	float w0 = glm::length(glm::cross(f1,f2)) / w;
	float w1 = glm::length(glm::cross(f2,f0)) / w;
	float w2 = glm::length(glm::cross(f0,f1)) / w;

	auto interpolatedNormal = v0n * w0 + v1n * w1 + v2n * w2;
	auto interpolatedUV = v0uv * w0 + v1uv * w1 + v2uv * w2;
	if(pUV)
		*pUV = interpolatedUV;

    return interpolatedNormal;
}

inline glm::vec3 glmmin3(const glm::vec3& a, const glm::vec3& b)
{
    return glm::vec3(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z)
    );
}

inline glm::vec3 glmmax3(const glm::vec3& a, const glm::vec3& b)
{
    return glm::vec3(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z)
    );
}

BBox Triangle::getBBox() const
{
    // Be Verbose!
    auto minLocale = glmmin3(glmmin3(v0,v1),v2);
    auto maxLocale = glmmax3(glmmax3(v0,v1),v2);

    return BBox(minLocale, maxLocale);
}

glm::vec3 Triangle::getCentroid() const
{
    return (v0 + v1 + v2) * 0.333f;
}