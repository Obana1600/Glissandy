#include "Extension.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;



Vector3 Normalize(const Vector3 &v) {
	float len = Length(v);

	if (len < 1e-6f) {
		return Vector3Zero();
	}

	return Vector3(v.x / len, v.y / len, v.z / len);
}


Vector4 operator+(const Vector4 &v) {
	return v;
}

Vector4 operator-(const Vector4 &v) {
	return Vector4(-v.x, -v.y, -v.z, -v.w);
}


Vector4 &operator+=(Vector4 &lhv, const Vector4 &rhv) {
	lhv = lhv + rhv;
	return lhv;
}

Vector4 &operator-=(Vector4 &lhv, const Vector4 &rhv) {
	lhv = lhv - rhv;
	return lhv;
}

Vector4 &operator*=(Vector4 &v, float s) {
	v = v * s;
	return v;
}

Vector4 &operator/=(Vector4 &v, float s) {
	if (s > 1e-6f) {
		v = v / s;
	}
	return v;
}


const Vector4 operator+(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

const Vector4 operator-(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

const Vector4 operator*(const Vector4 &v, float s) {
	return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
}

const Vector4 operator*(float s, const Vector4 &v) {
	return v * s;
}

const Vector4 operator/(const Vector4 &v, float s) {
	if (s < 1e-6f) {
		return v;
	}
	return Vector4(v.x / s, v.y / s, v.z / s, v.w / s);
}