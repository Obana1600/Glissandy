#include "TimeT.h"


void TimeT::Update() {
	
		const auto now = std::chrono::steady_clock::now();

		if (!isInitialized_) {
			lastTime_ = now;
			isInitialized_ = true;
			return;
		}

		deltaTime_ = std::chrono::duration<float>(now - lastTime_).count();

		// 目標値に近ければ丸める
		if (std::abs(deltaTime_ - kTarget_) < 0.002f) {
			deltaTime_ = kTarget_;
		}

		// 10FPS以下はクランプ
		if (deltaTime_ > 0.1f) {
			deltaTime_ = 0.1f;
		}

		time_ += deltaTime_;
		frameCount_++;
		lastTime_ = now;
	
}


float TimeT::GetFramerate() {
	return 1.0f / deltaTime_;
}


float TimeT::GetDeltaTimeMS() {
	return deltaTime_ * 1000.0f;
}