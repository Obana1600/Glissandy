#include "EaseManager.h"
#include "EaseFunc.h"


float EaseManager::EvaluateUnclamped(const TweenBase &t, float time, float duration) {
	float easeVal = Evaluate(t.easeType_, time, duration);

	if (t.loopType_ == LoopType::Incremental) {
		easeVal += t.IsComplete() ? static_cast<float>(t.completedLoops_ - 1) : static_cast<float>(t.completedLoops_);
	}

	return easeVal;
}


float EaseManager::Evaluate(EaseType easeType, float time, float duration) {
	float t = time / duration;

	switch (easeType) {
		case EaseType::Linear:
			return EaseFunc::Linear(t);

		case EaseType::InSine:
			return EaseFunc::InSine(t);
		case EaseType::OutSine:
			return EaseFunc::OutSine(t);
		case EaseType::InOutSine:
			return EaseFunc::InOutSine(t);

		case EaseType::InQuad:
			return EaseFunc::InQuad(t);
		case EaseType::OutQuad:
			return EaseFunc::OutQuad(t);
		case EaseType::InOutQuad:
			return EaseFunc::InOutQuad(t);

		case EaseType::InCubic:
			return EaseFunc::InCubic(t);
		case EaseType::OutCubic:
			return EaseFunc::OutCubic(t);
		case EaseType::InOutCubic:
			return EaseFunc::InOutCubic(t);

		case EaseType::InQuart:
			return EaseFunc::InQuart(t);
		case EaseType::OutQuart:
			return EaseFunc::OutQuart(t);
		case EaseType::InOutQuart:
			return EaseFunc::InOutQuart(t);

		case EaseType::InQuint:
			return EaseFunc::InQuint(t);
		case EaseType::OutQuint:
			return EaseFunc::OutQuint(t);
		case EaseType::InOutQuint:
			return EaseFunc::InOutQuint(t);

		case EaseType::InExpo:
			return EaseFunc::InExpo(t);
		case EaseType::OutExpo:
			return EaseFunc::OutExpo(t);
		case EaseType::InOutExpo:
			return EaseFunc::InOutExpo(t);

		case EaseType::InCirc:
			return EaseFunc::InCirc(t);
		case EaseType::OutCirc:
			return EaseFunc::OutCirc(t);
		case EaseType::InOutCirc:
			return EaseFunc::InOutCirc(t);

		case EaseType::InElastic:
			return EaseFunc::InElastic(t);
		case EaseType::OutElastic:
			return EaseFunc::OutElastic(t);
		case EaseType::InOutElastic:
			return EaseFunc::InOutElastic(t);

		case EaseType::InBack:
			return EaseFunc::InBack(t);
		case EaseType::OutBack:
			return EaseFunc::OutBack(t);
		case EaseType::InOutBack:
			return EaseFunc::InOutBack(t);

		case EaseType::InBounce:
			return EaseFunc::InBounce(t);
		case EaseType::OutBounce:
			return EaseFunc::OutBounce(t);
		case EaseType::InOutBounce:
			return EaseFunc::InOutBounce(t);

		case EaseType::Random:
			return EaseFunc::Random(t);

		default:
			return EaseFunc::Linear(t);
	}
}