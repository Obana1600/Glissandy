#pragma once

#include "TweenBase.h"


class EaseManager {
public: // メンバ関数

	static float EvaluateUnclamped(const TweenBase &t, float time, float duration);

	static float Evaluate(EaseType easeType, float time, float duration);
};