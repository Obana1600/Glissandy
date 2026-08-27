#include "Collision.h"

#include <algorithm>
#include <limits>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;


namespace {

	struct Range {
		float tMin;
		float tMax;
	};

	const float kInfinity = std::numeric_limits<float>::infinity();

	Range MakeEmptyRange() {
		return Range{kInfinity, -kInfinity};
	}

	Range ComputeRange(const AABB &aabb, const Vector3 &origin, const Vector3 &diff) {
		Range result{-kInfinity, kInfinity};

		for (int i = 0; i < 3; i++) {
			if (std::abs(diff[i]) < 1e-6f) {
				if (origin[i] < aabb.min[i] || origin[i] > aabb.max[i]) {
					return MakeEmptyRange();
				}
				continue;
			}

			float t1 = (aabb.min[i] - origin[i]) / diff[i];
			float t2 = (aabb.max[i] - origin[i]) / diff[i];

			result.tMin = (std::max)(result.tMin, (std::min)(t1, t2));
			result.tMax = (std::min)(result.tMax, (std::max)(t1, t2));
		}

		return result;
	}


	Matrix4x4 MakeWorldMatrixOBB(const OBB &obb) {
		return {{
			{obb.orientation[0].x, obb.orientation[0].y, obb.orientation[0].z, 0.0f},
			{obb.orientation[1].x, obb.orientation[1].y, obb.orientation[1].z, 0.0f},
			{obb.orientation[2].x, obb.orientation[2].y, obb.orientation[2].z, 0.0f},
			{obb.center.x, obb.center.y, obb.center.z, 1.0f},
		}};
	}

} // namespace


bool IsCollision(const Sphere &s1, const Sphere &s2) {
	// 2つの球の中心点間の距離を求める
	float dist = Length(s1.center - s2.center);
	// 半径の合計よりも短ければ衝突
	return dist <= s1.radius + s2.radius;
}


bool IsCollision(const Sphere &sphere, const Plane &plane) {
	float len = Length(plane.normal);
	if (len == 0.0f) {
		return false;
	}

	float k = std::abs(Dot(plane.normal, sphere.center) - plane.distance) / len; // 後から正規化

	return k <= sphere.radius;
}


bool IsCollision(const Plane &plane, const Sphere &sphere) {
	return IsCollision(sphere, plane);
}


bool IsCollision(const Line &line, const Plane &plane) {
	float proj = Dot(line.diff, plane.normal);
	if (proj == 0.0f) {
		float dist = plane.distance - Dot(line.origin, plane.normal);
		return dist == 0.0f; // 平面上にあるかチェック
	}

	return true;
}


bool IsCollision(const Plane &plane, const Line &line) {
	return IsCollision(line, plane);
}


bool IsCollision(const Ray &ray, const Plane &plane) {
	float den = Dot(ray.diff, plane.normal);
	float num = plane.distance - Dot(ray.origin, plane.normal);
	if (den == 0.0f) {
		return num == 0.0f; // 平面上にあるかチェック
	}

	return 0.0f <= num / den;
}


bool IsCollision(const Plane &plane, const Ray &ray) {
	return IsCollision(ray, plane);
}


bool IsCollision(const Segment &segment, const Plane &plane) {
	float den = Dot(segment.diff, plane.normal);
	float num = plane.distance - Dot(segment.origin, plane.normal);
	if (den == 0.0f) {
		return num == 0.0f; // 平面上にあるかチェック
	}

	float t = num / den;

	return 0.0f <= t && t <= 1.0f;
}


bool IsCollision(const Plane &plane, const Segment &segment) {
	return IsCollision(segment, plane);
}


bool IsCollision(const Triangle &triangle, const Segment &segment) {
	Vector3 v01 = triangle.vertices[1] - triangle.vertices[0];
	Vector3 v12 = triangle.vertices[2] - triangle.vertices[1];
	Vector3 v20 = triangle.vertices[0] - triangle.vertices[2];
	Vector3 n = Cross(v01, v12);

	float dist = Dot(n, triangle.vertices[0]);
	float den = Dot(segment.diff, n);
	if (den == 0.0f) {
		return false;
	}

	float t = (dist - Dot(segment.origin, n)) / den;
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	Vector3 p = segment.origin + t * segment.diff;
	Vector3 cross01 = Cross(v01, p - triangle.vertices[0]);
	Vector3 cross12 = Cross(v12, p - triangle.vertices[1]);
	Vector3 cross20 = Cross(v20, p - triangle.vertices[2]);

	return Dot(cross01, n) >= 0.0f && Dot(cross12, n) >= 0.0f && Dot(cross20, n) >= 0.0f;
}


bool IsCollision(const Segment &segment, const Triangle &triangle) {
	return IsCollision(triangle, segment);
}


bool IsCollision(const AABB &aabb1, const AABB &aabb2) {
	return aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z;
}


bool IsCollision(const AABB &aabb, const Sphere &sphere) {
	// 最近接点を求める
	Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
	};
	// 最近接点と球の中心との距離を求める
	float diff = Length(closestPoint - sphere.center);
	// 距離が半径よりも小さければ衝突
	return diff <= sphere.radius;
}


bool IsCollision(const Sphere &sphere, const AABB &aabb) {
	return IsCollision(aabb, sphere);
}


bool IsCollision(const AABB &aabb, const Line &line) {
	auto range = ComputeRange(aabb, line.origin, line.diff);
	// 交差していれば衝突
	return range.tMin <= range.tMax;
}


bool IsCollision(const Line &line, const AABB &aabb) {
	return IsCollision(aabb, line);
}


bool IsCollision(const AABB &aabb, const Ray &ray) {
	auto range = ComputeRange(aabb, ray.origin, ray.diff);
	// 交差していて、始点より前にあれば衝突
	return range.tMin <= range.tMax && 0.0f <= range.tMax;
}


bool IsCollision(const Ray &ray, const AABB &aabb) {
	return IsCollision(aabb, ray);
}


bool IsCollision(const AABB &aabb, const Segment &segment) {
	auto range = ComputeRange(aabb, segment.origin, segment.diff);
	// 交差していて、segmentの範囲[0,1]と重なっていれば衝突
	return range.tMin <= range.tMax && 0.0f <= range.tMax && range.tMin <= 1.0f;
}


bool IsCollision(const Segment &segment, const AABB &aabb) {
	return IsCollision(aabb, segment);
}


bool IsCollision(const OBB &obb, const Sphere &sphere) {
	// obbのワールド行列
	Matrix4x4 obbWorldMatrix = MakeWorldMatrixOBB(obb);

	// obbのローカル空間に変換 = obbをaabbに変換
	Vector3 centerInOBBLocalSpace = Transform(sphere.center, Inverse(obbWorldMatrix));

	// ローカル空間のaabbと球を求める
	AABB aabbOBBLocal{.min = -obb.size, .max = obb.size};
	Sphere sphereOBBLocal{centerInOBBLocalSpace, sphere.radius};

	return IsCollision(aabbOBBLocal, sphereOBBLocal);
}


bool IsCollision(const Sphere &sphere, const OBB &obb) {
	return IsCollision(obb, sphere);
}


bool IsCollision(const OBB &obb, const Line &line) {
	// obbのワールド行列
	Matrix4x4 obbWorldMatrix = MakeWorldMatrixOBB(obb);
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	// obbのローカル空間に変換 = obbをaabbに変換
	Vector3 localOrigin = Transform(line.origin, obbWorldMatrixInverse);
	Vector3 localEnd = Transform(line.origin + line.diff, obbWorldMatrixInverse);

	// ローカル空間のaabbとlineを求める
	AABB localAABB{.min = -obb.size, .max = obb.size};
	Line localLine{.origin = localOrigin, .diff = localEnd - localOrigin};

	return IsCollision(localAABB, localLine);
}


bool IsCollision(const Line &line, const OBB &obb) {
	return IsCollision(obb, line);
}


bool IsCollision(const OBB &obb, const Ray &ray) {
	// obbのワールド行列
	Matrix4x4 obbWorldMatrix = MakeWorldMatrixOBB(obb);
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	// obbのローカル空間に変換 = obbをaabbに変換
	Vector3 localOrigin = Transform(ray.origin, obbWorldMatrixInverse);
	Vector3 localEnd = Transform(ray.origin + ray.diff, obbWorldMatrixInverse);

	// ローカル空間のaabbとrayを求める
	AABB localAABB{.min = -obb.size, .max = obb.size};
	Ray localRay{.origin = localOrigin, .diff = localEnd - localOrigin};

	return IsCollision(localAABB, localRay);
}


bool IsCollision(const Ray &ray, const OBB &obb) {
	return IsCollision(obb, ray);
}


bool RaycastOBB(const OBB &obb, const Segment &segment, float *outT) {
	// obbのワールド行列
	Matrix4x4 obbWorldMatrix = MakeWorldMatrixOBB(obb);
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	// obbのローカル空間に変換 = obbをaabbに変換
	Vector3 localOrigin = Transform(segment.origin, obbWorldMatrixInverse);
	Vector3 localEnd = Transform(segment.origin + segment.diff, obbWorldMatrixInverse);

	// ローカル空間のaabbとsegmentを求める
	AABB localAABB{.min = -obb.size, .max = obb.size};
	Range range = ComputeRange(localAABB, localOrigin, localEnd - localOrigin);

	// segmentの条件
	if (range.tMin > range.tMax || range.tMax < 0.0f || range.tMin > 1.0f) {
		return false;
	}

	if (outT) {
		*outT = (std::max)(range.tMin, 0.0f);
	}

	return true;
}


bool IsCollision(const OBB &obb, const Segment &segment) {
	return RaycastOBB(obb, segment, nullptr);
}


bool IsCollision(const Segment &segment, const OBB &obb) {
	return RaycastOBB(obb, segment, nullptr);
}


bool IsCollision(const OBB &obb1, const OBB &obb2) {
	// 各obbの頂点を求める
	Vector3 verticesOBB1[8]{}, verticesOBB2[8]{};
	uint32_t index = 0;
	for (float x : {-1.0f, 1.0f}) {
		for (float y : {-1.0f, 1.0f}) {
			for (float z : {-1.0f, 1.0f}) {
				verticesOBB1[index] = obb1.center + obb1.orientation[0] * obb1.size.x * x + obb1.orientation[1] * obb1.size.y * y + obb1.orientation[2] * obb1.size.z * z;
				verticesOBB2[index] = obb2.center + obb2.orientation[0] * obb2.size.x * x + obb2.orientation[1] * obb2.size.y * y + obb2.orientation[2] * obb2.size.z * z;
				index++;
			}
		}
	}

	// 分離軸axisに対してobbを射影し、分離している（衝突していない）かを返すラムダ
	auto isSeparated = [&](const Vector3 &axis) {
		// axis
		float len = Length(axis);
		if (len == 0.0f) {
			return false; // 分離軸がゼロベクトルならfalse
		}
		Vector3 norm = axis / len;

		// 点をaxisへ射影した長さの最低値、最大値を求める
		float min1 = Dot(verticesOBB1[0], norm);
		float max1 = min1;
		float min2 = Dot(verticesOBB2[0], norm);
		float max2 = min2;
		for (uint32_t i = 1; i < 8; i++) { // [1]から始める
			float dot1 = Dot(verticesOBB1[i], norm);
			float dot2 = Dot(verticesOBB2[i], norm);

			min1 = (std::min)(min1, dot1);
			max1 = (std::max)(max1, dot1);
			min2 = (std::min)(min2, dot2);
			max2 = (std::max)(max2, dot2);
		}

		// 影の長さの合計を求める
		float L1 = max1 - min1;
		float L2 = max2 - min2;
		// 2つの影の両端の差分を求める
		float span = (std::max)(max1, max2) - (std::min)(min1, min2);

		// 分離しているならtrueを返す
		return L1 + L2 < span;
	};

	// 面法線で判定
	for (uint32_t i = 0; i < 3; i++) {
		if (isSeparated(obb1.orientation[i])) {
			return false;
		}
	}
	for (uint32_t i = 0; i < 3; i++) {
		if (isSeparated(obb2.orientation[i])) {
			return false;
		}
	}

	// クロス積で判定
	for (uint32_t i = 0; i < 3; i++) {
		for (uint32_t j = 0; j < 3; j++) {
			if (isSeparated(Cross(obb1.orientation[i], obb2.orientation[j]))) {
				return false;
			}
		}
	}

	// すべての判定がtrueなら衝突している
	return true;
}


bool IsCollision(const Capsule &capsule, const Plane &plane) {
	if (IsCollision(capsule.segment, plane)) {
		return true;
	}

	Sphere sphereStart = {capsule.segment.origin, capsule.radius};
	Sphere sphereEnd = {capsule.segment.origin + capsule.segment.diff, capsule.radius};

	return IsCollision(sphereStart, plane) || IsCollision(sphereEnd, plane);
}


bool IsCollision(const Plane &plane, const Capsule &capsule) {
	return IsCollision(capsule, plane);
}