#include "Geometry.h"
#include <algorithm>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


// linear -------------------------------------------------------------------------------------

Vector3 Project(const Vector3 &v1, const Vector3 &v2) {
	auto copyV2 = v2;
	Vector3 normV2 = Normalize(copyV2);
	return Dot(v1, normV2) * normV2;
}


Vector3 ClosestPoint(const Vector3 &point, const Segment &segment) {
	float den = Dot(segment.diff, segment.diff);
	if (den == 0.0f) {
		return segment.origin;
	}

	float t = Dot(point - segment.origin, segment.diff) / den;
	t = std::clamp(t, 0.0f, 1.0f); // segmentなのでclampする

	return segment.origin + t * segment.diff;
}

// --------------------------------------------------------------------------------------------


// plane --------------------------------------------------------------------------------------

Vector3 Perpendicular(const Vector3 &v) {
	if (v.x != 0.0f || v.y != 0.0f) {
		return {-v.y, v.x, 0.0f};
	}
	return {0.0f, -v.z, v.y};
}

// --------------------------------------------------------------------------------------------


Vector3 Reflect(const Vector3 &input, const Vector3 &normal) {
	return input - 2.0f * (Dot(input, normal)) * normal;
}