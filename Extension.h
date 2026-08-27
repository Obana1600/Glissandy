#pragma once

#include "KamataEngine.h"


KamataEngine::Vector3 Normalize(const KamataEngine::Vector3 &v);



KamataEngine::Vector4 operator+(const KamataEngine::Vector4 &v);
KamataEngine::Vector4 operator-(const KamataEngine::Vector4 &v);

KamataEngine::Vector4 &operator+=(KamataEngine::Vector4 &lhv, const KamataEngine::Vector4 &rhv);
KamataEngine::Vector4 &operator-=(KamataEngine::Vector4 &lhv, const KamataEngine::Vector4 &rhv);
KamataEngine::Vector4 &operator*=(KamataEngine::Vector4 &v, float s);
KamataEngine::Vector4 &operator/=(KamataEngine::Vector4 &v, float s);

const KamataEngine::Vector4 operator+(const KamataEngine::Vector4 &v1, const KamataEngine::Vector4 &v2);
const KamataEngine::Vector4 operator-(const KamataEngine::Vector4 &v1, const KamataEngine::Vector4 &v2);
const KamataEngine::Vector4 operator*(const KamataEngine::Vector4 &v, float s);
const KamataEngine::Vector4 operator*(float s, const KamataEngine::Vector4 &v);
const KamataEngine::Vector4 operator/(const KamataEngine::Vector4 &v, float s);