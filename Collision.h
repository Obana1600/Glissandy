#pragma once

#include "Geometry.h"


// sphere vs sphere
bool IsCollision(const Sphere &s1, const Sphere &s2);

// sphere vs plane
bool IsCollision(const Sphere &sphere, const Plane &plane);
bool IsCollision(const Plane &plane, const Sphere &sphere);

// line vs plane
bool IsCollision(const Line &line, const Plane &plane);
bool IsCollision(const Plane &plane, const Line &line);

// ray vs plane
bool IsCollision(const Ray &ray, const Plane &plane);
bool IsCollision(const Plane &plane, const Ray &ray);

// segment vs plane
bool IsCollision(const Segment &segment, const Plane &plane);
bool IsCollision(const Plane &plane, const Segment &segment);

// triangle vs segment
bool IsCollision(const Triangle &triangle, const Segment &segment);
bool IsCollision(const Segment &segment, const Triangle &triangle);

// aabb vs aabb
bool IsCollision(const AABB &aabb1, const AABB &aabb2);

// aabb vs sphere
bool IsCollision(const AABB &aabb, const Sphere &sphere);
bool IsCollision(const Sphere &sphere, const AABB &aabb);

// aabb vs line
bool IsCollision(const AABB &aabb, const Line &line);
bool IsCollision(const Line &line, const AABB &aabb);

// aabb vs ray
bool IsCollision(const AABB &aabb, const Ray &ray);
bool IsCollision(const Ray &ray, const AABB &aabb);

// aabb vs segment
bool IsCollision(const AABB &aabb, const Segment &segment);
bool IsCollision(const Segment &segment, const AABB &aabb);

// obb vs sphere
bool IsCollision(const OBB &obb, const Sphere &sphere);
bool IsCollision(const Sphere &sphere, const OBB &obb);

// line vs obb
bool IsCollision(const OBB &obb, const Line &line);
bool IsCollision(const Line &line, const OBB &obb);

// ray vs obb
bool IsCollision(const OBB &obb, const Ray &ray);
bool IsCollision(const Ray &ray, const OBB &obb);

// segment vs obb
bool RaycastOBB(const OBB &obb, const Segment &segment, float *outT);
bool IsCollision(const OBB &obb, const Segment &segment);
bool IsCollision(const Segment &segment, const OBB &obb);

// obb vs obb
bool IsCollision(const OBB &obb1, const OBB &obb2);

// capsule vs plane
bool IsCollision(const Capsule &capsule, const Plane &plane);
bool IsCollision(const Plane &plane, const Capsule &capsule);